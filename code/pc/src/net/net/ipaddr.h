/**
 * @file ipaddr.h
 * @author lishutong (527676163@qq.com)
 * @brief IP地址定义及接口函数
 * @version 0.1
 * @date 2022-10-26
 *
 * @copyright Copyright (c) 2022
 */
#ifndef IPADDR_H
#define IPADDR_H

#include <stdint.h>
#include "net_err.h"

#define IPV4_ADDR_BROADCAST       0xFFFFFFFF  // 广播地址
#define IPV4_ADDR_SIZE             4            // IPv4地址长度

/**
 * @brief IP地址
 */
typedef struct _ipaddr_t {
    enum {
        IPADDR_V4,
    }type;              // 地址类型

    union {
        // 注意，IP地址总是按大端存放
        uint32_t q_addr;                        // 32位整体描述
        uint8_t a_addr[IPV4_ADDR_SIZE];        // 数组描述
    };
}ipaddr_t;

void ipaddr_set_any(ipaddr_t * ip);
net_err_t ipaddr_from_str(ipaddr_t * dest, const char* str);
ipaddr_t * ipaddr_get_any(void);
void ipaddr_copy(ipaddr_t * dest, const ipaddr_t * src);
int ipaddr_is_equal(const ipaddr_t * ipaddr1, const ipaddr_t * ipaddr2);
void ipaddr_to_buf(const ipaddr_t* src, uint8_t* ip_buf);
void ipaddr_from_buf(ipaddr_t* dest, const uint8_t * ip_buf);
int ipaddr_is_local_broadcast(const ipaddr_t * ipaddr);
int ipaddr_is_direct_broadcast(const ipaddr_t * ipaddr, const ipaddr_t * netmask);
int ipaddr_is_any(const ipaddr_t* ip);
ipaddr_t ipaddr_get_net(const ipaddr_t * ipaddr, const ipaddr_t * netmask);
int ipaddr_is_match(const ipaddr_t* dest, const ipaddr_t* src, const ipaddr_t * netmask);
void ipaddr_set_all_1(ipaddr_t* ip);
int ipaddr_1_cnt(ipaddr_t* ip);
void ipaddr_set_loop (ipaddr_t * ipaddr);

#endif // ipaddr_H
