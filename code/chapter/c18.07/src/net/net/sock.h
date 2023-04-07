/**
 * @file sock.h
 * @author lishutong(527676163@qq.com)
 * @brief 数据通信sock实现
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 * 提供基本的sock结构，用于实现RAW/TCP/UDP等接口的操作的底层支持
 */
#ifndef SOCK_H
#define SOCK_H

#include "exmsg.h"

/**
 * @brief sock创建消息
 */
typedef struct _sock_create_t {
    int family;
    int protocol;
	int type;
}sock_create_t;

/**
 * @brief TCP API消息
 */
typedef struct _sock_req_t {
    int sockfd;
    union {
        sock_create_t create;
    };
}sock_req_t;

net_err_t sock_create_req_in(func_msg_t* api_msg);

net_err_t socket_init(void);

#endif // SOCK_H
