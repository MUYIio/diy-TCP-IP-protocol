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
#ifndef ARP_H
#define ARP_H

#include "pktbuf.h"
#include "netif.h"
#include "ether.h"
#include "ipaddr.h"

#define ARP_HW_ETHER            0x1             // 以太网类型
#define ARP_REQUEST             0x1             // ARP请求包
#define ARP_REPLY               0x2             // ARP响应包

/**
 * @brief ARP包
 */
#pragma pack(1)
typedef struct _arp_pkt_t {
    // type和len字段用于使包可用于不同类型的硬件层和协议层
    // 在我们这里，固定只支持MAC和IP转换，所以写死处理
    uint16_t htype;         // 硬件类型
    uint16_t ptype;         // 协议类型
    uint8_t hlen;           // 硬件地址长
    uint8_t plen;           // 协议地址长
    uint16_t opcode;        // 请求/响应
    uint8_t send_haddr[ETH_HWA_SIZE];       // 发送包硬件地址
    uint8_t send_paddr[IPV4_ADDR_SIZE];     // 发送包协议地址
    uint8_t target_haddr[ETH_HWA_SIZE];     // 接收方硬件地址
    uint8_t target_paddr[IPV4_ADDR_SIZE];   // 接收方协议地址
}arp_pkt_t;

#pragma pack()

/**
 * @brief ARP缓存表项
 */
typedef struct _arp_entry_t {
    uint8_t paddr[IPV4_ADDR_SIZE];      // 协议地址，即IP地址, 大端格式的ip地址?
    uint8_t haddr[ETH_HWA_SIZE];        // 硬件地址，即mac地址

    // 状态及标志位
    enum {
        NET_ARP_FREE = 0x1234,          // 空闲
        NET_ARP_RESOLVED,               // 稳定，已解析
        NET_ARP_WAITING,                // 挂起，有请求，但未解析成功
    } state;            // 状态位

    int tmo;                // 超时，需删除或重新请求
    int retry;              // 请求重试次数，因目标主机可能暂时未能处理，或丢包
    netif_t* netif;         // 包项所对应的网络接口，可用于发包
    nlist_node_t node;       // 下一个表项链接结点
    nlist_t buf_list;        // 待发送的数据包队列
}arp_entry_t;

net_err_t arp_init (void);
net_err_t arp_make_request(netif_t* netif, const ipaddr_t* pro_addr);
net_err_t arp_make_gratuitous(netif_t* netif);
net_err_t arp_in(netif_t* netif, pktbuf_t * buf);
net_err_t arp_make_reply(netif_t* netif, pktbuf_t* buf);
net_err_t arp_resolve(netif_t* netif, const ipaddr_t* ipaddr, pktbuf_t* buf);
const uint8_t* arp_find(netif_t* netif, ipaddr_t* ip_addr);
void arp_clear(netif_t * netif);
void arp_update_from_ipbuf(netif_t * netif, pktbuf_t * buf);

#endif // ARP_H
