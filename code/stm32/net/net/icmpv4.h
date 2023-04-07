/**
 * @file icmpv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>
#include "netif.h"
#include "pktbuf.h"

/**
 * @brief ICMP包类型
 */
typedef enum _icmp_type_t {
    ICMPv4_ECHO_REPLY = 0,                  // 回送响应
    ICMPv4_ECHO_REQUEST = 8,                // 回送请求
    ICMPv4_UNREACH = 3,                     // 不可达
}icmp_type_t;

/**
 * @brief ICMP包代码
 */
typedef enum _icmp_code_t {
    ICMPv4_ECHO = 0,                        // echo的响应码
    ICMPv4_UNREACH_PRO = 2,                 // 协议不可达
    ICMPv4_UNREACH_PORT = 3,                // 端口不可达
}icmp_code_t;

#pragma pack(1)

/**
 * @brief 包头：含类型、代码和校验和
 */
typedef struct _icmpv4_hdr_t {
    uint8_t type;           // 类型
    uint8_t code;			// 代码
    uint16_t checksum;	    // ICMP报文的校验和
}icmpv4_hdr_t;

/**
 * ICMP报文
 */
typedef struct _icmpv4_pkt_t {
    icmpv4_hdr_t hdr;            // 头和数据区
    union {
        uint32_t reverse;       // 保留项
    };
    uint8_t data[1];            // 可选数据区
}icmpv4_pkt_t;

#pragma pack()

net_err_t icmpv4_init(void);
net_err_t icmpv4_in(ipaddr_t *src_ip, ipaddr_t* netif_ip, pktbuf_t *buf);
net_err_t icmpv4_out_unreach(ipaddr_t* dest_addr, ipaddr_t* src, uint8_t code, pktbuf_t* ip_buf);

#endif // ICMP_H
