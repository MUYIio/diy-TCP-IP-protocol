/**
 * @file raw.h
 * @author lishutong(527676163@qq.com)
 * @brief 原始套接字
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 */
#ifndef RAW_H
#define RAW_H

#include "sock.h"

/**
 * 原始套接字结构
 */
typedef struct _raw_t {
    sock_t base;                // 基础sock块
    sock_wait_t rcv_wait;       // 接收等待结构
}raw_t;

net_err_t raw_init(void);
sock_t* raw_create(int family, int protocol);

#endif // RAW_H
