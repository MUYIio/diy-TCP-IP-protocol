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

    netif_t* netif;         // 包项所对应的网络接口，可用于发包
    nlist_node_t node;       // 下一个表项链接结点
    nlist_t buf_list;        // 待发送的数据包队列
}arp_entry_t;

net_err_t arp_init (void);

#endif // ARP_H
