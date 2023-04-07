﻿/**
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

struct _sock_t;
struct x_sockaddr;

typedef int x_socklen_t;			// 地址长度类型

/**
 * @brief 基本的操作接口
 * 这些接口会用来承接socket接口的底层具体功能实现
 */
typedef struct _sock_ops_t {
	net_err_t (*close)(struct _sock_t* s);
	net_err_t (*sendto)(struct _sock_t * s, const void* buf, size_t len, int flags,
                        const struct x_sockaddr* dest, x_socklen_t dest_len, ssize_t * result_len);
	net_err_t(*recvfrom)(struct _sock_t* s, void* buf, size_t len, int flags,
                        struct x_sockaddr* src, x_socklen_t * addr_len, ssize_t * result_len);
	net_err_t (*setopt)(struct _sock_t* s,  int level, int optname, const char * optval, int optlen);
    void (*destroy)(struct _sock_t *s);
}sock_ops_t;

/**
 * @brief sock结构
 */
typedef struct _sock_t {
	ipaddr_t local_ip;				// 源IP地址
	ipaddr_t remote_ip;				// 目的IP地址
	uint16_t local_port;			// 源端口
	uint16_t remote_port;			// 目的端口
	const sock_ops_t* ops;			// 具体的操作接口
    int family;                     // 协议簇
	int protocol;					// 对应的协议类型
	int err;						// 上一次操作的错误码
	int rcv_tmo;					// 接收超时，以毫秒计
	int snd_tmo;					// 发送超时，以毫秒计

	nlist_node_t node;				// 插入sock链表的连接接口
}sock_t;

/**
 * @brief sock创建消息
 */
typedef struct _sock_create_t {
    int family;
    int protocol;
	int type;
}sock_create_t;

/**
 * @brief 数据传送消息
 */
typedef struct _sock_data_t {
    uint8_t * buf;
    size_t len;
    int flags;
    struct x_sockaddr* addr;
    x_socklen_t * addr_len;
    ssize_t comp_len;
}sock_data_t;

/**
 * @brief TCP API消息
 */
typedef struct _sock_req_t {
    int sockfd;
    union {
        sock_create_t create;
        sock_data_t data;
    };
}sock_req_t;

net_err_t sock_init(sock_t* sock, int family, int protocol, const sock_ops_t * ops);
void sock_uninit (sock_t * sock);

net_err_t sock_create_req_in(func_msg_t* api_msg);
net_err_t sock_sendto_req_in (func_msg_t * api_msg);
net_err_t sock_recvfrom_req_in(func_msg_t * api_msg);

net_err_t socket_init(void);

#endif // SOCK_H
