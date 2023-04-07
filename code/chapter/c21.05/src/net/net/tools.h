/**
 * @file tools.h
 * @author lishutong (527676163@qq.com)
 * @brief net工具集
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 * 包含字节顺序转换函数、校验和计算等功能的支持
 */
#ifndef TOOLS_H
#define TOOLS_H

#include "net_plat.h"
#include "ipaddr.h"
#include "pktbuf.h"
#include "dbg.h"

/**
 * @brief 交换16位的字节顺序
 */
static inline uint16_t swap_u16(uint16_t v) {
    // 7..0 => 15..8, 15..8 => 7..0
    uint16_t r = ((v & 0xFF) << 8) | ((v >> 8) & 0xFF);
    return r;
}

/**
 * @brief 交换32位的字节顺序
 */
static inline uint32_t swap_u32(uint32_t v) {
    uint32_t r =
              (((v >>  0) & 0xFF) << 24)    // 0..7 -> 24..31
            | (((v >>  8) & 0xFF) << 16)    // 8..15 -> 16..23
            | (((v >> 16) & 0xFF) << 8)     // 16..23 -> 8..15
            | (((v >> 24) & 0xFF) << 0);    // 24..31 -> 0..7
    return r;
}

#if NET_ENDIAN_LITTLE       // 小端模式，需要转换
uint16_t swap_u16(uint16_t v);
uint32_t swap_u32(uint32_t v);
#define x_htons(v)        swap_u16(v)
#define x_ntohs(v)        swap_u16(v)
#define x_htonl(v)        swap_u32(v)
#define x_ntohl(v)        swap_u32(v)

#else
// 大端模式，保持不变
#define x_htons(v)        (v)
#define x_ntohs(v)        (v)
#define x_htonl(v)        (v)
#define x_ntohl(v)        (v)
#endif

net_err_t tools_init(void);
uint16_t checksum16(uint32_t offset, void* buf, uint16_t len, uint32_t pre_sum, int complement);
uint16_t checksum_peso(const uint8_t* src_ip, const uint8_t* dest_ip, uint8_t protocol, pktbuf_t* buf);

#endif // TOOLS_H
