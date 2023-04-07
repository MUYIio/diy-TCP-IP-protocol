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

    // 没有用定长消息队列，目的是减少开销。定长消息队列要求有缓存表
    // 再者还会创建多个信号量等结构，资源开锁大一些
    nlist_t recv_list;           // 接收队列
    sock_wait_t rcv_wait;       // 接收等待结构
}raw_t;

net_err_t raw_init(void);
sock_t* raw_create(int family, int protocol);
net_err_t raw_in(pktbuf_t* pktbuf);

#endif // RAW_H
