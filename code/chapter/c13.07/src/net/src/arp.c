/**
 * @file arp.h
 * @author lishutong (527676163@qq.com)
 * @brief ARP协议支持
 * @version 0.1
 * @date 2022-10-29
 * 
 * @copyright Copyright (c) 2022
 * 
 * IPv4协议使用的是IPv4地址，其目标是设计跨越不同类型物理网络的分组交换。
 * 而以太网使用的是mac地址，需要使用正确的mac地址才能保证对方接受到数据包。
 * 因此，使用ARP协议(RFC826)用于在IPv4地址和网络硬件地址进行转换。
 * 
 * ARP协议被设计成兼容多种不同类型的物理网络，但实际总是被用于IPv4和mac地址之间的转换
 * IPv6不使用ARP协议，而使用ICMPv6中的邻居发现协议。
 * 另外还有一种RARP协议，用于缺少磁盘的系统，现在几乎很少使用
 */
#include <string.h>
#include "arp.h"
#include "net_cfg.h"
#include "mblock.h"
#include "dbg.h"
#include "nlocker.h"
#include "protocol.h"
#include "pktbuf.h"
#include "tools.h"
#include "ipaddr.h"

static arp_entry_t cache_tbl[ARP_CACHE_SIZE];    // arp缓存
static mblock_t cache_mblock;                // 空闲arp分配结构
static nlist_t cache_list;                   // 动态表
static const uint8_t empty_hwaddr[] = {0, 0, 0, 0, 0, 0};   // 空闲硬件地址

#if DBG_DISP_ENABLED(DBG_ARP)

/**
 * @brief 打印已解析的arp表项
 */
void display_arp_entry(arp_entry_t* entry) {
    plat_printf("%d: ", (int)(entry - cache_tbl));       // 序号
    dump_ip_buf(" ip:", entry->paddr);
    dump_mac(" mac:", entry->haddr);
    plat_printf(" tmo: %d, retry: %d, %s, buf: %d\n",
        entry->tmo, entry->retry, entry->state == NET_ARP_RESOLVED ? "stable" : "pending",
        nlist_count(&entry->buf_list));
}

/**
 * @brief 显示ARP表中所有项
 */
void display_arp_tbl(void) {
    plat_printf("\n------------- ARP table start ---------- \n");

    arp_entry_t* entry = cache_tbl;
    for (int i = 0; i < ARP_CACHE_SIZE; i++, entry++) {
        if ((entry->state != NET_ARP_FREE) && (entry->state != NET_ARP_RESOLVED)) {
            continue;
        }

        display_arp_entry(entry);
    }

    plat_printf("------------- ARP table end ---------- \n");
}

/**
 * 打印ARP包的完整类型
 */
static void arp_pkt_display(arp_pkt_t* packet) {
    uint16_t opcode = x_ntohs(packet->opcode);

    plat_printf("--------------- arp start ------------------\n");
    plat_printf("    htype:%x\n", x_ntohs(packet->htype));
    plat_printf("    pype:%x\n", x_ntohs(packet->ptype));
    plat_printf("    hlen: %x\n", packet->hlen);
    plat_printf("    plen:%x\n", packet->plen);
    plat_printf("    type:%04x  ", opcode);
    switch (opcode) {
    case ARP_REQUEST:
        plat_printf("request\n");
        break;;
    case ARP_REPLY:
        plat_printf("reply\n");
        break;
    default:
        plat_printf("unknown\n");
        break;
    }
    dump_ip_buf("    sender:", packet->send_paddr);
    dump_mac("  mac:", packet->send_haddr);
    plat_printf("\n");
    dump_ip_buf("    target:", packet->target_paddr);
    dump_mac("  mac:", packet->target_haddr);
    plat_printf("\n");
    plat_printf("--------------- arp end ------------------ \n");
}

#else
#define display_arp_entry(entry)
#define display_arp_tbl()
#define arp_pkt_display(packet)
#endif

/**
 * @brief 初始化ARP缓存表及链表
 * ARP缓存用于维护每个接口中得到的IPv4地址到mac地址之间的转换关系
 */
static net_err_t cache_init(void) {
    nlist_init(&cache_list);
    
    // 建立缓存链
    plat_memset(cache_tbl, 0, sizeof(cache_tbl));
    net_err_t err = mblock_init(&cache_mblock, cache_tbl, sizeof(arp_entry_t), ARP_CACHE_SIZE, NLOCKER_NONE);
    if (err < 0) {
        return err;
    }

    return NET_ERR_OK;
}

/**
 * @brief ARP模块初始化
 */
net_err_t arp_init(void) {
    // ARP缓存初始化
    net_err_t err = cache_init();
    if (err < 0) {
        dbg_error(DBG_ARP, "arp cache init failed.");
        return err;
    }

    return NET_ERR_OK;
}


/**
 * @brief 向指定网口上发送ARP查询请求
 */
net_err_t arp_make_request(netif_t* netif, const ipaddr_t* pro_addr) {
    // 分配ARP包的空间
    pktbuf_t* buf = pktbuf_alloc(sizeof(arp_pkt_t));
    if (buf == NULL) {
        dbg_dump_ip(DBG_ARP, "allocate arp packet failed. ip:", pro_addr);
        return NET_ERR_NONE;
    }

    // 这里看起来似乎什么必要，不过还是加上吧
    pktbuf_set_cont(buf, sizeof(arp_pkt_t));

    // 填充ARP包，请求包，写入本地的ip和mac、目标Ip，目标硬件写空值
    arp_pkt_t* arp_packet = (arp_pkt_t*)pktbuf_data(buf);
    arp_packet->htype = x_htons(ARP_HW_ETHER);
    arp_packet->ptype = x_htons(NET_PROTOCOL_IPv4);
    arp_packet->hlen = ETH_HWA_SIZE;
    arp_packet->plen = IPV4_ADDR_SIZE;
    arp_packet->opcode = x_htons(ARP_REQUEST);
    plat_memcpy(arp_packet->send_haddr, netif->hwaddr.addr, ETH_HWA_SIZE);
    ipaddr_to_buf(&netif->ipaddr, arp_packet->send_paddr);
    plat_memcpy(arp_packet->target_haddr, empty_hwaddr, ETH_HWA_SIZE);
    ipaddr_to_buf(pro_addr, arp_packet->target_paddr);

    // 调试输出报告
    arp_pkt_display(arp_packet);

    // 发广播通知所有主机
    net_err_t err = ether_raw_out(netif, NET_PROTOCOL_ARP, ether_broadcast_addr(), buf);
    if (err < 0) {
        pktbuf_free(buf);
    }
    return err;
}

/**
 * @brief 创建无回报(免费)的ARP(ARP通告，告知所有主机自己使用这个IP地址
 * 用于网卡启动时主动向外界报告自己的mac，这样别的主机可先更新，下次直接使用数据
 * 包中的mac地址与自己通信，避免发送ARP请求包
 * 此外，当网络中有其它主机使用了同样的IP地址时，可收到来自对方的响应。
 * 不过，协议对此不做任何处理。像win上只是简单的提示IP地址冲突
 */
net_err_t arp_make_gratuitous(netif_t* netif) {
    dbg_info(DBG_ARP, "send an gratuitous arp....");

    // 无回报ARP包是目标地址为本网卡IP的请求包, 目标mac为空
    // ip地址不能为空，也不能为其它主机的ip，所以只能填自己的ip
    return arp_make_request(netif, &netif->ipaddr);
}

/**
 * @brief 对接收到的ARP请求包进行回应
 * 简单起见，直接用原请求包进行修改，然后回发
 */
net_err_t arp_make_reply(netif_t* netif, pktbuf_t* buf) {
    arp_pkt_t* arp_packet = (arp_pkt_t*)pktbuf_data(buf);

    // 用原来的send地址，填充到target地址区, 再将send地址，填入自己的，同时写入自己的mac
    arp_packet->opcode = x_htons(ARP_REPLY);
    plat_memcpy(arp_packet->target_haddr, arp_packet->send_haddr, ETH_HWA_SIZE);
    plat_memcpy(arp_packet->target_paddr, arp_packet->send_paddr, IPV4_ADDR_SIZE);
    plat_memcpy(arp_packet->send_haddr, netif->hwaddr.addr, ETH_HWA_SIZE);
    ipaddr_to_buf(&netif->ipaddr, arp_packet->send_paddr);

    // 调试输出报告
    arp_pkt_display(arp_packet);
    return ether_raw_out(netif, NET_PROTOCOL_ARP, arp_packet->target_haddr, buf);
}

/**
 * @brief 检查包是否有错误
 */
static net_err_t is_pkt_ok(arp_pkt_t* arp_packet, uint16_t size, netif_t* netif) {
    if (size < sizeof(arp_pkt_t)) {
        dbg_warning(DBG_ARP, "packet size error: %d < %d", size, (int)sizeof(arp_pkt_t));
        return NET_ERR_SIZE;
    }

    // 上层协议和硬件类型不同的要丢掉
    if ((x_ntohs(arp_packet->htype) != ARP_HW_ETHER) ||
        (arp_packet->hlen != ETH_HWA_SIZE) ||
        (x_ntohs(arp_packet->ptype) != NET_PROTOCOL_IPv4) ||
        (arp_packet->plen != IPV4_ADDR_SIZE)) {
        dbg_warning(DBG_ARP, "packet incorrect");
        return NET_ERR_NOT_SUPPORT;
    }

    // 可能还有RARP等类型，全部丢掉
    uint32_t opcode = x_ntohs(arp_packet->opcode);
    if ((opcode != ARP_REQUEST) && (opcode != ARP_REPLY)) {
        dbg_warning(DBG_ARP, "unknown opcode=%d", arp_packet->opcode);
        return NET_ERR_NOT_SUPPORT;
    }

    return NET_ERR_OK;
}

/**
 * 处理接收到的arp请求或响应包
 * 只对目的地的IP地址和自己的IP地址完全一样的包进行响应，多播、广播IP无响应
 * 并且不扫描自己的其它网络接口地址，因为即便是将其硬件地址发回。对方使用该硬件地址
 * 通信时，由于大概率不是在同一物理网络，所以也是收不到的。
 * @param netif 收到数据包的接口
 * @param buf 收到的数据包，由neif传进来
 */
net_err_t arp_in(netif_t* netif, pktbuf_t* buf) {
    dbg_info(DBG_ARP, "arp in");

    // 调整包头位置，保证连续性，方便接下来的处理
    net_err_t err = pktbuf_set_cont(buf, sizeof(arp_pkt_t));
    if (err < 0) {
        return err;
    }

    // 对包头部做一些必要性的检查
    arp_pkt_t * arp_packet = (arp_pkt_t*)pktbuf_data(buf);
    if (is_pkt_ok(arp_packet, buf->total_size, netif) != NET_ERR_OK) {
        return err;
    }

    // 如果是对方发来的请求，响应之
    if (x_ntohs(arp_packet->opcode) == ARP_REQUEST) {
        dbg_info(DBG_ARP, "arp is request. try to send reply");
        return arp_make_reply(netif, buf);
    }

    display_arp_tbl();

    // 注意要释放掉包!!!
    pktbuf_free(buf);
    return NET_ERR_OK;
}