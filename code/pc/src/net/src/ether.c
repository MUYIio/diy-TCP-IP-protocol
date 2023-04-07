/**
 * @file ether.h
 * @author lishutong (527676163@qq.com)
 * @brief 以太网协议支持，不包含ARP协议处理
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <string.h>
#include "netif.h"
#include "ether.h"
#include "dbg.h"
#include "pktbuf.h"
#include "protocol.h"
#include "tools.h"
#include "arp.h"
#include "ipv4.h"

/**
 * @brief 打印以太网数据头信息
 */
#if DBG_DISP_ENABLED(DBG_ETHER)
static void display_ether_display(char * title, ether_pkt_t * pkt, int size) {
    ether_hdr_t * hdr = (ether_hdr_t *)pkt;

    plat_printf("\n--------------- %s ------------------ \n", title);
    plat_printf("\tlen: %d bytes\n", size);
    dump_mac("\tdest:", hdr->dest);
    dump_mac("\tsrc:", hdr->src);
    plat_printf("\ttype: %04x - ", x_ntohs(hdr->protocol));
    switch (x_ntohs(hdr->protocol)) {
    case NET_PROTOCOL_ARP:
        plat_printf("ARP\n");
        break;
    case NET_PROTOCOL_IPv4:
        plat_printf("IP\n");
        break;
    default:
        plat_printf("Unknown\n");
        break;
    }
    plat_printf("\n");
}

#else
#define display_ether_display(title, pkt, size)
#endif

/**
 * @brief 对指定接口设备进行以太网协议相关初始化
 */
static net_err_t ether_open(netif_t* netif) {
    // 接口打开时需要发送无回报的arp包
    return arp_make_gratuitous(netif);
}

/**
 * @brief 以太网的关闭
 */
static void ether_close(netif_t* netif) {
    arp_clear(netif);
}

/**
 * 检查包是否有错误
 */
static net_err_t is_pkt_ok(ether_pkt_t * frame, int total_size) {
    if (total_size > (sizeof(ether_hdr_t) + ETH_MTU)) {
        dbg_warning(DBG_ETHER, "frame size too big: %d", total_size);
        return NET_ERR_SIZE;
    }

    // 虽然以太网规定最小60字节，但是底层驱动收到可能小于这么多
    if (total_size < (sizeof(ether_hdr_t))) {
        dbg_warning(DBG_ETHER, "frame size too small: %d", total_size);
        return NET_ERR_SIZE;
    }

    return NET_ERR_OK;
}

/**
 * 以太网输入包的处理
 */
net_err_t ether_in(netif_t* netif, pktbuf_t* buf) {
    dbg_info(DBG_ETHER, "ether in:");

    // 似乎没什么必要，必须是连续的，但清空是加上。
    pktbuf_set_cont(buf, sizeof(ether_hdr_t));

    ether_pkt_t* pkt = (ether_pkt_t*)pktbuf_data(buf);

    // 检查包的正确性
    net_err_t err;
    if ((err = is_pkt_ok(pkt, buf->total_size)) != NET_ERR_OK) {
        dbg_error(DBG_ETHER, "ether pkt error");
        return err;
    }

    // 显示帧，方式调试观察
    display_ether_display("ether in", pkt, buf->total_size);

    // 各种协议处理，交给ARP或IP
    switch (x_ntohs(pkt->hdr.protocol)) {
    case NET_PROTOCOL_ARP: {
        // 调整偏移量，跳过以太网包头
        err = pktbuf_remove_header(buf, sizeof(ether_hdr_t));
        if (err < 0) {
            dbg_error(DBG_ETHER, "remove ether header failed, packet size: %d", buf->total_size);
            return NET_ERR_SIZE;
        }

        return arp_in(netif, buf);;
    }
    case NET_PROTOCOL_IPv4: {
        // 收到发来给自己的IP包或者广播包，则用其中的mac和ARP更新，因为后续
        // 很大可能自己会需要同对方进行通信，避免了再发ARP请求
        arp_update_from_ipbuf(netif, buf);

        // 调整偏移量，跳过以太网包头
        err = pktbuf_remove_header(buf, sizeof(ether_hdr_t));
        if (err < 0) {
            dbg_error(DBG_ETHER, "remove ethernet header failed, packet size: %d", buf->total_size);
            return NET_ERR_SIZE;
        }

        // 加入IP处理
        err = ipv4_in(netif, buf);
        if (err < 0) {
            dbg_warning(DBG_ETHER, "process in buf failed. err=%d", err);
            return err;
        }
        break;
    }
    default:
        dbg_warning(DBG_ETHER, "unknown packet, ignore it.");
        return NET_ERR_NOT_SUPPORT;
    }

    return NET_ERR_OK;
}

/**
 * @brief 获取以太网广播地址
 */
const uint8_t * ether_broadcast_addr(void) {
    static const uint8_t broadcast_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return broadcast_addr;
}

/**
 * @brief 发送一个原始的以太网数据包，不经过ARP请求
 * 该发送不使用arp，要求直接填入目标mac地址
 */
net_err_t ether_raw_out(netif_t* netif, uint16_t protocol, const uint8_t* dest, pktbuf_t* buf) {
    net_err_t err;

    // 以太网最小帧长度有要求，这里进行调整，使得驱动就不用关心这个问题
    int size = pktbuf_total(buf);
    if (size < ETH_DATA_MIN) {
        dbg_info(DBG_ETHER, "resize from %d to %d", size, (int)ETH_DATA_MIN);

        // 调整至46字节
        err = pktbuf_resize(buf, ETH_DATA_MIN);
        if (err < 0) {
            dbg_error(DBG_ETHER, "resize failed: %d", err);
            return err;
        }

        // 填充新增的部分为0
        pktbuf_reset_acc(buf);
        pktbuf_seek(buf, size);
        pktbuf_fill(buf, 0, ETH_DATA_MIN - size);
    }

    // 调整读写位置，预留以太网包头，并设置为连续位置
    err = pktbuf_add_header(buf, sizeof(ether_hdr_t), 1);
    if (err < 0) {
        dbg_error(DBG_ETHER, "add header failed: %d", err);
        return NET_ERR_SIZE;
    }

    // 填充以太网帧头，发送
    ether_pkt_t * pkt = (ether_pkt_t*)pktbuf_data(buf);
    plat_memcpy(pkt->hdr.dest, dest, ETH_HWA_SIZE);
    plat_memcpy(pkt->hdr.src, netif->hwaddr.addr, ETH_HWA_SIZE);
    pkt->hdr.protocol = x_htons(protocol);        // 注意大小端转换

    // 显示包信息
    display_ether_display("ether out", pkt, size);

    // 如果是发给自己的，那么直接写入发送队列。
    // 例如，应用层可能ping本机某个网卡的IP，显然不能将这个发到网络上
    // 而是应当插入到发送队列中。可以考虑在IP模块中通过判断IP地址来处理，也可以像这样
    // 在底层处理。当然，这样处理的效率会低一些
    if (plat_memcmp(netif->hwaddr.addr, dest, ETH_HWA_SIZE) == 0) {
       return netif_put_in(netif, buf, -1);
    } else {
        err = netif_put_out(netif, buf, -1);
        if (err < 0) {
            dbg_warning(DBG_ETHER, "put pkt out failed: %d", err);
            return err;
        }

        // 通知网卡开始发送
        return netif->ops->xmit(netif);
    }
}

/**
 * 发送一个ip数据包
 * 在发送时会先查询arp表，如果有表项，是发送；没有则先请求arp进行解析
 */
static net_err_t ether_out(netif_t* netif, ipaddr_t* ip_addr, pktbuf_t* buf) {
    // 发给网卡自己的，不用查ARP，走近写到输入队列里
    if (ipaddr_is_equal(&netif->ipaddr, ip_addr)) {
        return ether_raw_out(netif, NET_PROTOCOL_IPv4, (const uint8_t *)netif->hwaddr.addr, buf);
    }

    // 外部的，发给网卡
    const uint8_t* hwaddr = arp_find(netif, ip_addr);
    if (hwaddr) {
        return ether_raw_out(netif, NET_PROTOCOL_IPv4, hwaddr, buf);
    } else {
        // 数据包会由arp模式后续自动发送
        return arp_resolve(netif, ip_addr, buf);
    }
}

/**
 * 初始化以太网接口层
 */
net_err_t ether_init(void) {
    static const link_layer_t link_layer = {
        .type = NETIF_TYPE_ETHER,
        .open = ether_open,
        .close = ether_close,
        .in = ether_in,
        .out = ether_out,
    };

    dbg_info(DBG_ETHER, "init ether");

    // 注册以太网驱链接层接口
    net_err_t err = netif_register_layer(NETIF_TYPE_ETHER, &link_layer);
    if (err < 0) {
        dbg_info(DBG_ETHER, "error = %d", err);
        return err;
    }

    dbg_info(DBG_ETHER, "done.");
    return NET_ERR_OK;
}
