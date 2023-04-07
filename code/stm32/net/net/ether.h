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
#ifndef ETHER_H
#define ETHER_H

#include "net_err.h"
#include "netif.h"

#define ETH_HWA_SIZE        6       // mac地址长度
#define ETH_MTU             1500    // 最大传输单元，数据区的大小
#define ETH_DATA_MIN        46      // 最小发送的帧长，即头部+46

#pragma pack(1)

/**
 * @brief 以太网帧头
 */
typedef struct _ether_hdr_t {
    uint8_t dest[ETH_HWA_SIZE];         // 目标mac地址
    uint8_t src[ETH_HWA_SIZE];          // 源mac地址
    uint16_t protocol;                  // 协议/长度
}ether_hdr_t;

/**
 * @brief 以太网帧格式
 * 肯定至少要求有1个字节的数据
 */
typedef struct _ether_pkt_t {
    ether_hdr_t hdr;                    // 帧头
    uint8_t data[ETH_MTU];              // 数据区
}ether_pkt_t;

#pragma pack()

const uint8_t* ether_broadcast_addr(void);
net_err_t ether_raw_out(netif_t* netif, uint16_t protocol, const uint8_t* dest, pktbuf_t* buf);
net_err_t ether_init(void);
net_err_t ether_in(netif_t* netif, pktbuf_t* packet);

#endif // ETHER_H
