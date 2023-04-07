/**
 * @file sock.c
 * @author lishutong(527676163@qq.com)
 * @brief 数据通信sock实现
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 * 提供基本的sock结构，用于实现RAW/TCP/UDP等接口的操作的底层支持
 */
#include "sock.h"
#include "socket.h"
#include "dbg.h"

#define SOCKET_MAX_NR		(RAW_MAX_NR)
static x_socket_t socket_tbl[SOCKET_MAX_NR];          // 空闲socket表

/**
 * @brief 将socket指针转换为socket索引
 */
static inline int get_index(x_socket_t* socket) {
    return (int)(socket - socket_tbl);
}

/**
 * @brief 将索引转换成结构
 */
static inline x_socket_t* get_socket(int idx) {
    // 做一些必要性的检查，以便参数不正确时，不用浪费消息传递
    if ((idx < 0) || (idx >= SOCKET_MAX_NR)) {
        return (x_socket_t*)0;
    }

    x_socket_t* s = socket_tbl + idx;
    return s;
}

/**
 * @brief 分配一个socket结构
 */
static x_socket_t * socket_alloc (void) {
    x_socket_t * s = (x_socket_t *)0;

    // 遍历整个列表，找空闲项
    for (int i = 0; i < SOCKET_MAX_NR; i++) {
        x_socket_t * curr = socket_tbl + i;
        if (curr->state == SOCKET_STATE_FREE) {
            s = curr;
            s->state = SOCKET_STATE_USED;
            break;
        }
    }

    return s;
}

/**
 * @brief 释放socket
 */
static void socket_free(x_socket_t* s) {
    s->state = SOCKET_STATE_FREE;
}

/**
 * @brief socket层初始化
 */
net_err_t socket_init(void) {
    plat_memset(socket_tbl, 0, sizeof(socket_tbl));
    return NET_ERR_OK;
}


/**
 * @brief 初始化sock结构
 * 根据不同的协议对sock做不同的初始化
 */
net_err_t sock_init(sock_t* sock, int family, int protocol, const sock_ops_t * ops) {
	sock->protocol = protocol;
	sock->ops = ops;

    sock->family = family;
	ipaddr_set_any(&sock->local_ip);
	ipaddr_set_any(&sock->remote_ip);
	sock->local_port = 0;
	sock->remote_port = 0;
	sock->err = NET_ERR_OK;
	sock->rcv_tmo = 0;
    sock->snd_tmo = 0;
	nlist_node_init(&sock->node);
	return NET_ERR_OK;
}

/**
 * @brief 回收释放sock相关资源，但不包含sock本身
 */
void sock_uninit (sock_t * sock) {
}

/**
 * @brief 创建一个tcp socket
 */
net_err_t sock_create_req_in(func_msg_t* api_msg) {
	sock_req_t * req = (sock_req_t *)api_msg->param;
	sock_create_t * param = &req->create;

    // 分配socket结构，建立连接
    x_socket_t * socket = socket_alloc();
    if (socket == (x_socket_t *)0) {
        dbg_error(DBG_SOCKET, "no socket");
        return NET_ERR_MEM;
    }

    req->sockfd = get_index(socket);
    return NET_ERR_OK;
}
