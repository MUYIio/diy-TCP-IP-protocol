/**
 * @file ipv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 * 
 * @copyright Copyright (c) 2022
 * 包含IP包输入输出处理，数据报分片与重组、路由表
 */
#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>
#include "net_err.h"

#define IPV4_ADDR_SIZE          4           // IPV4地址长度

#pragma pack(1)

/**
 * @brief IP包头部
 * 长度可变，最少20字节，如下结构。当包含选项时，长度变大，最多60字节
 * 不过，包含选项的包比较少见，因此一般为20字节
 * 总长最大65536，实际整个pktbuf的长度可能比total_len大，因为以太网填充的缘故
 * 因此需要total_len来判断实际有效的IP数据包长
 */
typedef struct _ipv4_hdr_t {
    uint16_t shdr_all;
    uint16_t total_len;		    // 总长度、
    uint16_t id;		        // 标识符，用于区分不同的数据报,可用于ip数据报分片与重组
    uint16_t frag_all;

    uint8_t ttl;                // 存活时间，每台路由器转发时减1，减到0时，该包被丢弃
    uint8_t protocol;	        // 上层协议
    uint16_t hdr_checksum;      // 首部校验和
    uint8_t	src_ip[IPV4_ADDR_SIZE];        // 源IP
    uint8_t dest_ip[IPV4_ADDR_SIZE];	   // 目标IP
}ipv4_hdr_t;

/**
 * @brief IP数据包
 */
typedef struct _ipv4_pkt_t {
    ipv4_hdr_t hdr;              // 数据包头
    uint8_t data[1];            // 数据区
}ipv4_pkt_t;

#pragma pack()

net_err_t ipv4_init(void);

#endif // IPV4_H
