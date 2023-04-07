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
#include "timer.h"
#include "ipv4.h"

// 将超时值转换为定时器的扫描次数
// 比如超时设置成10s，表本身的扫描次数为2次，则该表项被扫描10 / 2次后，将会超时
#define to_scan_cnt(tmo)     (tmo / ARP_TIMER_TMO)

static net_timer_t cache_timer;                       // arp表扫描定时器
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
 * @brief 释放entry中所有等待的数据包
 */
static void cache_clear_all(arp_entry_t* entry) {
    dbg_info(DBG_ARP, "clear %d packet:", nlist_count(&entry->buf_list));
    dbg_dump_ip_buf(DBG_ARP, "ip:", entry->paddr);
    dbg_dump_mac(DBG_ARP, "mac:", entry->haddr);

    nlist_node_t * first;
    while ((first = nlist_remove_first(&entry->buf_list))) {
        pktbuf_t* buf = nlist_entry(first, pktbuf_t, node);
        pktbuf_free(buf);
    }
}

/**
 * 将entry上所有等待发送的数据包全部发送出去
 */
static net_err_t cache_send_all (arp_entry_t* entry) {
    dbg_info(DBG_ARP, "send %d packet:", nlist_count(&entry->buf_list));
    dbg_dump_ip_buf(DBG_ARP, "ip:", entry->paddr);
    dbg_dump_mac(DBG_ARP, "mac:", entry->haddr);

    nlist_node_t * first;
    while ((first = nlist_remove_first(&entry->buf_list))) {
        pktbuf_t* buf = nlist_entry(first, pktbuf_t, node);

        // 将数据包通过以太网发送出去，发往指定的mac地址
        net_err_t err = ether_raw_out(entry->netif, NET_PROTOCOL_IPv4, entry->haddr, buf);
        if (err < 0) {
            // 发送成功时，由底层释放，失败由自己失败
            pktbuf_free(buf);
            return err;
        }
    }

    return  NET_ERR_OK;
}

/**
 * @brief 分配一个空闲或可用的ARP表项
 */
static arp_entry_t* cache_alloc(int force) {
    arp_entry_t* entry = mblock_alloc(&cache_mblock, -1);
    if (!entry && force) {
        // 找不到空闲的项，且强制分配，则从已有的列表中找时间最久的（处于表尾）
        // 因为每次扫描表查找表项时，找到后都会将表项插入到表的末尾
        nlist_node_t * node = nlist_remove_last(&cache_list);
        if (!node) {
            dbg_warning(DBG_ARP, "allocate arp entry failed");
            return (arp_entry_t*)0;
        }

        dbg_info(DBG_ARP, "allocate an arp entry from cache list");

        // 释放掉表项上的所有数据包
        entry = nlist_entry(node, arp_entry_t, node);
        cache_clear_all(entry);
    }

    if (entry) {
        // 分配得到合适的项，对必要的项初始化
        plat_memset(entry, 0, sizeof(arp_entry_t));
        entry->state = NET_ARP_FREE;
        nlist_node_init(&entry->node);
        nlist_init(&entry->buf_list);
    }
    return entry;
}

/**
 * @brief 释放ARP表项
 */
static void cache_free(arp_entry_t *entry) {
    // 需要将所有的包发出去，再修改状态
    cache_clear_all(entry);

    // 从cache表中移除，再释放资源
    nlist_remove(&cache_list, &entry->node);
    mblock_free(&cache_mblock, entry);
}

/**
 * 以指定的pro_addr查找对应的ARP表项
 */
static arp_entry_t* cache_find(uint8_t* ip) {
    nlist_node_t* node;
    nlist_for_each(node, &cache_list) {
        arp_entry_t* entry = nlist_entry(node, arp_entry_t, node);
        if (plat_memcmp(ip, entry->paddr, IPV4_ADDR_SIZE) == 0) {
            // 从表中移除，然后插入到表头，这样可使得最新使用的表项位于表头
            // 这样下次大概率也会使用该表项，以便节省下次查找的时间
            nlist_remove(&cache_list, node);
            nlist_insert_first(&cache_list, node);
            return entry;
        }
    }

    return (arp_entry_t*)0;
}

/**
 * 更新ARP表项，用于在接受到ARP表项时处理
 */
static void cache_entry_set(arp_entry_t* entry, const uint8_t* hwaddr,
                uint8_t* proaddr, netif_t* netif, int state) {
    plat_memcpy(entry->haddr, hwaddr, ETH_HWA_SIZE);
    plat_memcpy(entry->paddr, proaddr, IPV4_ADDR_SIZE);
    entry->state = state;
    entry->netif = netif;

    if (state == NET_ARP_RESOLVED) {
        entry->tmo = to_scan_cnt(ARP_ENTRY_STABLE_TMO);         // 重启定时
    } else {
        entry->tmo = to_scan_cnt(ARP_ENTRY_PENDING_TMO);        // 重启定时
    }
    entry->retry = ARP_ENTRY_RETRY_CNT;
}

/**
 * @brief 用指定的地址对来更新ARP表。如果表项不存在，则新分配一个表项
 * 收到的包总是包含有发送包的ip和mac，意味着可以总是更新表项或新建表项
 */
static net_err_t cache_insert(netif_t* netif, uint8_t* pro_addr, uint8_t* hw_addr, int force) {
    // 查找已有的缓存
    arp_entry_t* entry = cache_find(pro_addr);
    if (!entry) {
        // 找不到，需分配一个表项
        entry = cache_alloc(force);
        if (!entry) {
            dbg_dump_ip_buf(DBG_ARP, "alloc failed! sender ip:", pro_addr);
            return NET_ERR_NONE;
        }

        // 更新表项，再插入到表头
        cache_entry_set(entry, hw_addr, pro_addr, netif, NET_ARP_RESOLVED);
        nlist_insert_first(&cache_list, &entry->node);
        dbg_dump_ip_buf(DBG_ARP, "insert an entry, sender ip:", pro_addr);
    }
    else {
        // 表中找到，则需要更新，移到头，因为可能接下来会立即用这个通信
        dbg_dump_ip_buf(DBG_ARP, "update arp entry， sender ip:", pro_addr);
        dbg_dump_mac(DBG_ARP, "sender mac:", hw_addr);
        cache_entry_set(entry, hw_addr, pro_addr, netif, NET_ARP_RESOLVED);
        if (nlist_first(&cache_list) != &entry->node) {
            nlist_remove(&cache_list, &entry->node);
            nlist_insert_first(&cache_list, &entry->node);
        }

        // 此时，由于表中已经有相应的mac地址，所以可以将所有挂载的数据包给发出去了
        net_err_t err = cache_send_all(entry);
        if (err < 0) {
            dbg_error(DBG_ARP, "send packet in entry failed. err = %d", err);
            return err;
        }
    }

    display_arp_tbl();
    return NET_ERR_OK;
}

/**
 * @brief 使用IP包来更新ARP表
 */
void arp_update_from_ipbuf(netif_t* netif, pktbuf_t* pkt) {
    // 单个buf肯定要至少容纳IP包
    net_err_t err = pktbuf_set_cont(pkt, sizeof(ipv4_hdr_t) + sizeof(ether_hdr_t));
    if (err < 0) {
        dbg_error(DBG_ARP, "adjust header failed. err = %d", err);
        return;
    }

    ether_hdr_t* eth_hdr = (ether_hdr_t*)pktbuf_data(pkt);
    ipv4_hdr_t* ip_hdr = (ipv4_hdr_t*)((uint8_t*)eth_hdr + sizeof(ether_hdr_t));

    // 进行必要的ip包检查，只处理ipv4
    if (ip_hdr->version != NET_VERSION_IPV4) {
        dbg_warning(DBG_ARP, "not ipv4, skip");
        return;
    }
    int total_size = x_ntohs(ip_hdr->total_len);
    if ((total_size < sizeof(ipv4_hdr_t)) || (pkt->total_size < total_size)) {
        dbg_warning(DBG_IP, "ip packet too small %d!\n", total_size);
        return;
    }

    // 仅处理发往自己的包：IP地址完全相同，或者是广播地址等
    ipaddr_t dest_ip;
    ipaddr_from_buf(&dest_ip, ip_hdr->dest_ip);
    if (ipaddr_is_match(&netif->ipaddr, &dest_ip, &netif->netmask)) {
        // 用ip包头和以太网包的的数据更新arp表
        cache_insert(netif, ip_hdr->src_ip, eth_hdr->src, 0);
    }
}

/**
 * @brief ARP定时处理
 * 减掉每个表项的tmo，一旦发现超时，则删除
 * 应对两种问题：
 * 1、发送ARP请求后无响应，需超时处理，以回收表项
 * 2、某个表项存在太久，没有更新。重新查询，以确认该表项仍然有效。因为有可能别的主机上线，占用了该ip
 * 而此时mac地址已经发生了变化
 */
static void arp_cache_tmo(net_timer_t* timer, void * arg) {
    int changed_cnt = 0;

    // 遍历所有的pending/stable列表，进行超时处理
    // static表，因为是手动绑定的，所以不应当扫描超时处理
    nlist_node_t* curr, * next;
    for (curr = cache_list.first; curr; curr = next) {
        arp_entry_t* entry = nlist_entry(curr, arp_entry_t, node);

        // 预先取下一结点，因为后面可能会删除当前结点，导致当前遍历过程出错
        next = nlist_node_next(curr);

        // 非超时，跳过
        if (--entry->tmo > 0) {
            continue;
        }

        changed_cnt++;
        switch (entry->state) {
            case NET_ARP_RESOLVED: {
                // 超时，切换至Pending状态，发送ARP请求
                dbg_info(DBG_ARP, "stable to pending:");
                display_arp_entry(entry);

                // 切换超时，重设相关时间，发包重新请求
                ipaddr_t ipaddr;
                ipaddr_from_buf(&ipaddr, entry->paddr);
                entry->state = NET_ARP_WAITING;
                entry->tmo = to_scan_cnt(ARP_ENTRY_PENDING_TMO);
                entry->retry = to_scan_cnt(ARP_ENTRY_RETRY_CNT);
                arp_make_request(entry->netif, &ipaddr);
                break;
            }
            case NET_ARP_WAITING: {
                if (--entry->retry == 0) {
                    dbg_info(DBG_ARP, "pending tmo, free it.");
                    display_arp_entry(entry);

                    // 超过重试次数，无响应，释放回收表项
                    cache_free(entry);
                } else {
                    dbg_info(DBG_ARP, "pending tmo, send request.");
                    display_arp_entry(entry);

                    // 还有重试次数，再次重发
                    ipaddr_t ipaddr;
                    ipaddr_from_buf(&ipaddr, entry->paddr);
                    entry->tmo = to_scan_cnt(ARP_ENTRY_PENDING_TMO);
                    arp_make_request(entry->netif, &ipaddr);
                }
                break;
            }
            default: {
                dbg_error(DBG_ARP, "unknown arp state.");
                display_arp_entry(entry);
                break;
            }
        }
    }

    // 有修改，打印一下，方便调试观察
    if (changed_cnt) {
        dbg_info(DBG_ARP, "%d arp entry changed.", changed_cnt);
        display_arp_tbl();
    }
}

/**
 * @brief 清除所有与netif相关的ARP表项
 */
void arp_clear(netif_t* netif) {
    // 遍历扫描
    nlist_node_t* node;
    for (node = nlist_first(&cache_list); node; node = node->next) {
        // 先取下一结点，因为后面可能删除
        nlist_node_t* next = nlist_node_next(node);

        // 碰到相同网络接口的，则移除掉
        arp_entry_t* e = nlist_entry(node, arp_entry_t, node);
        if (e->netif == netif) {
            cache_clear_all(e);
            nlist_remove(&cache_list, node);

            mblock_free(&cache_mblock, e);
        }

        node = next;
    }
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

    // 创建定时器后启动
    err = net_timer_add(&cache_timer, "arp timer", arp_cache_tmo, (void *)0, ARP_TIMER_TMO *1000, NET_TIMER_RELOAD);
    if (err < 0) {
        dbg_error(DBG_ARP, "create timer failed: %d.", err);
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

    arp_pkt_display(arp_packet);

    // 判断目的包是否是自己
    ipaddr_t target_ip;
    ipaddr_from_buf(&target_ip, arp_packet->target_paddr);
    if (ipaddr_is_equal(&target_ip, &netif->ipaddr)) {
        dbg_info(DBG_ARP, "received an arp for me, force update.");

        // 1.自己主动通信，先发起ARP 2、对方主动通信，先发起ARP
        // 不管是请求还是响应包，目标Ip都是自己，很有可能要与自己通信。
        // 用包中的源mac和ip先更新表，而且强制更新(这是一种优化策略)
        cache_insert(netif, arp_packet->send_paddr, arp_packet->send_haddr, 1);

        // 如果是对方发来的请求，响应之
        if (x_ntohs(arp_packet->opcode) == ARP_REQUEST) {
            dbg_info(DBG_ARP, "arp is request. try to send reply");
            return arp_make_reply(netif, buf);
        }
    } else {
        dbg_info(DBG_ARP, "received an arp not for me, try to update.");

        // 目标ip不是自己的，检查ARP表，更新已有的表项。但不强制，有ARP表空项就更新
        cache_insert(netif, arp_packet->send_paddr, arp_packet->send_haddr, 0);
    }

    // 注意要释放掉包!!!
    pktbuf_free(buf);
    return NET_ERR_OK;
}

/**
 * 根据ip地址找相应的表项
 * TODO: 可以和arp_resolve进行合并
 */
const uint8_t* arp_find(netif_t* netif, ipaddr_t* ip) {
    // 广播地址，转换为硬件广播直接发送
    if (ipaddr_is_local_broadcast(ip) || ipaddr_is_direct_broadcast(ip, &netif->netmask)) {
        return ether_broadcast_addr();
    }

    // 仅查找ip地址完全相同的表项
    arp_entry_t* entry = cache_find(ip->a_addr);
    if (entry && (entry->state == NET_ARP_RESOLVED)) {
        return entry->haddr;
    }

    return (const uint8_t*)0;
}

/**
 * 当表项不存在或者正处于Pending状态时，将包插入到ARP解析表后，发送ARP请求包去查询
 */
net_err_t arp_resolve(netif_t* netif, const ipaddr_t* ipaddr, pktbuf_t* buf) {
    // 转换ip地址
    uint8_t pro_addr[IPV4_ADDR_SIZE];
    ipaddr_to_buf(ipaddr, pro_addr);

    // 非广播地址，则查找ARP表
    arp_entry_t * entry = cache_find(pro_addr);
    if (entry) {
        dbg_info(DBG_ARP, "found an arp entry.");

        // 如果找到表项且为已解释，则将包发送出去
        if (entry->state == NET_ARP_RESOLVED) {
            net_err_t err = ether_raw_out(entry->netif, NET_PROTOCOL_IPv4, entry->haddr, buf);
            return err;
        }

        // 未解析的表项，加入到发送队列，但如果队列已满则不用加入
        if (nlist_count(&entry->buf_list) <= ARP_MAX_PKT_WAIT) {
            dbg_info(DBG_ARP, "insert packet to arp entry");
            nlist_insert_first(&entry->buf_list, &buf->node);
            return NET_ERR_OK;
        } else {
            dbg_warning(DBG_ARP, "too many waiting. ignore it");
            return NET_ERR_FULL;
        }
    }  else {
        dbg_dump_ip(DBG_ARP, "make arp request, ip:", ipaddr);

        // 如果找不到，则考虑分配一个
        entry = cache_alloc(1);
        if (entry == (arp_entry_t*)0) {
            dbg_error(DBG_ARP, "alloc arp failed.");
            return NET_ERR_MEM;
        }

        // 初始化清空mac，插入表头，因为下次可能优先查询同样的pro_addr
        cache_entry_set(entry, empty_hwaddr, pro_addr, netif, NET_ARP_WAITING);
        nlist_insert_first(&cache_list, &entry->node);

        dbg_info(DBG_ARP, "insert packet to arp");
        nlist_insert_last(&entry->buf_list, &buf->node);

        // 有修改表，打印调试
        display_arp_tbl();

        // 由于是新分配的表，发送ARP请求
        return arp_make_request(netif, ipaddr);
    }
}
