﻿/**
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
#include "raw.h"

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
 * @brief 初始化等待结构
 */
net_err_t sock_wait_init (sock_wait_t * wait) {
    wait->waiting = 0;
    wait->sem = sys_sem_create(0);
    return wait->sem == SYS_SEM_INVALID ? NET_ERR_SYS : NET_ERR_OK;
}

/**
 * @brief 销毁等待结构
 */
void sock_wait_destroy (sock_wait_t * wait) {
    if (wait->sem != SYS_SEM_INVALID) {
        sys_sem_free(wait->sem);
    }
}

/**
 * @brief 进入等待状态
 */
net_err_t sock_wait_enter (sock_wait_t * wait, int tmo) {
    if (sys_sem_wait(wait->sem, tmo) < 0) {
	    return NET_ERR_TMO;
    }

    return wait->err;
}

/**
 * @brief 设置需要等待
 * @param wait
 */
void sock_wait_add (sock_wait_t * wait, int tmo, struct _sock_req_t * req) {
    req->wait = wait;
    req->wait_tmo = tmo;
    wait->waiting++;
}

/**
 * @brief 退出等待状态
 */
void sock_wait_leave (sock_wait_t * wait, net_err_t err) {
    if (wait->waiting > 0) {
        wait->waiting--;
        wait->err = err;
        sys_sem_notify(wait->sem);
    }
}

/**
 * @brief 通知指定类型的任务某种事件发生(err指定)
 */
void sock_wakeup (sock_t * sock, int type, int err) {
    if (type & SOCK_WAIT_CONN) {
        sock_wait_leave(sock->conn_wait, err);
    }

    if (type & SOCK_WAIT_WRITE) {
        sock_wait_leave(sock->snd_wait, err);
    }

    if (type & SOCK_WAIT_READ) {
        sock_wait_leave(sock->rcv_wait, err);
    }
    sock->err = err;
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

    sock->conn_wait = (sock_wait_t *)0;
    sock->snd_wait = (sock_wait_t *)0;
    sock->rcv_wait = (sock_wait_t *)0;
	return NET_ERR_OK;
}

/**
 * @brief 回收释放sock相关资源，但不包含sock本身
 */
void sock_uninit (sock_t * sock) {
    if (sock->snd_wait) {
	    sock_wait_destroy(sock->snd_wait);
    }

    if (sock->rcv_wait) {
        sock_wait_destroy(sock->rcv_wait);
    }

    if (sock->conn_wait) {
        sock_wait_destroy(sock->conn_wait);
    }
}

/**
 * @brief 创建一个tcp socket
 */
net_err_t sock_create_req_in(func_msg_t* api_msg) {
    // 根据不同的协议创建不同的sock信息表
    static const struct sock_info_t {
        int protocol;			// 缺省的协议
        sock_t* (*create) (int family, int protocol);
    }  sock_tbl[] = {
        [SOCK_RAW] = { .protocol = 0, .create = raw_create,}
    };

	sock_req_t * req = (sock_req_t *)api_msg->param;
	sock_create_t * param = &req->create;

    // 分配socket结构，建立连接
    x_socket_t * socket = socket_alloc();
    if (socket == (x_socket_t *)0) {
        dbg_error(DBG_SOCKET, "no socket");
        return NET_ERR_MEM;
    }

    // 对type参数进行检查
    if ((param->type < 0) || (param->type >= sizeof(sock_tbl) / sizeof(sock_tbl[0]))) {
        dbg_error(DBG_SOCKET, "unknown type: %d", param->type);
        socket_free(socket);
        return NET_ERR_PARAM;
    }

    // 根据协议，创建底层sock, 未指定协议，使用缺省的
    const struct sock_info_t* info = sock_tbl + param->type;
    if (param->protocol == 0) {
        param->protocol = info->protocol;
    }

	// 根据类型创建不同的socket
	sock_t * sock = info->create(param->family, param->protocol);
	if (!sock) {
        dbg_error(DBG_SOCKET, "create sock failed, type: %d", param->type);
        socket_free(socket);
		return NET_ERR_MEM;
	}

    socket->sock = sock;
    req->sockfd = get_index(socket);
    return NET_ERR_OK;
}

/**
 * @brief 发送数据包
 * 这样RAW和UDP就不用再实现自己的send接口，直接用这个就可以了
 */
net_err_t sock_sendto_req_in (func_msg_t * api_msg) {
	sock_req_t * req = (sock_req_t *)api_msg->param;
    x_socket_t* s = get_socket(req->sockfd);
    if (!s) {
        dbg_error(DBG_SOCKET, "param error: socket = %d.", s);
        return NET_ERR_PARAM;
    }
    sock_t* sock = s->sock;
	sock_data_t * data = (sock_data_t *)&req->data;

    // 判断是否已经实现
    if (!sock->ops->sendto) {
        dbg_error(DBG_SOCKET, "this function is not implemented");
        return NET_ERR_NOT_SUPPORT;
    }

    net_err_t err = sock->ops->sendto(sock, data->buf, data->len, data->flags,
                                data->addr, *data->addr_len, &req->data.comp_len);
    if (err == NET_ERR_NEED_WAIT) {
        if (sock->snd_wait) {
            sock_wait_add(sock->snd_wait, sock->snd_tmo, req);
        }
    }
    return err;
}

/**
 * @brief 接收数据包
 */
net_err_t sock_recvfrom_req_in(func_msg_t * api_msg) {
	sock_req_t * req = (sock_req_t *)api_msg->param;
    x_socket_t* s = get_socket(req->sockfd);
    if (!s) {
        dbg_error(DBG_SOCKET, "param error: socket = %d.", s);
        return NET_ERR_PARAM;
    }
    sock_t* sock = s->sock;
	sock_data_t * data = (sock_data_t *)&req->data;


    // 判断是否已经实现
    if (!sock->ops->recvfrom) {
        dbg_error(DBG_SOCKET, "this function is not implemented");
        return NET_ERR_NOT_SUPPORT;
    }

	net_err_t err = sock->ops->recvfrom(sock, data->buf, data->len, data->flags,
                        data->addr, data->addr_len, &req->data.comp_len);
    if (err == NET_ERR_NEED_WAIT) {
        if (sock->rcv_wait) {
            sock_wait_add(sock->rcv_wait, sock->rcv_tmo, req);
        }
    }
    return err;
}

/**
 * @brief 设置opt选项消息
 */
net_err_t sock_setsockopt_req_in(func_msg_t * api_msg) {
    return NET_ERR_OK;
}
