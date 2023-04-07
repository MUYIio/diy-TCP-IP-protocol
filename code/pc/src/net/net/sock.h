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

struct _sock_t;
struct x_sockaddr;
struct _sock_req_t;

typedef int x_socklen_t;			// 地址长度类型

// TCP等待类型
#define SOCK_WAIT_READ         (1 << 0)        
#define SOCK_WAIT_WRITE        (1 << 1)       
#define SOCK_WAIT_CONN         (1 << 2)     
#define SOCK_WAIT_ALL          (SOCK_WAIT_CONN |SOCK_WAIT_READ | SOCK_WAIT_WRITE) 

/**
 * @brief TCP等待结构
 */
typedef struct _sock_wait_t {
    net_err_t err;                  // 等待结果
    int waiting;                    // 是否有任务等待
    sys_sem_t sem;                  // 事件完成的信号量
}sock_wait_t;

net_err_t sock_wait_init (sock_wait_t * wait);
void sock_wait_destroy (sock_wait_t * wait);
void sock_wait_add (sock_wait_t * wait, int tmo, struct _sock_req_t * req);
net_err_t sock_wait_enter (sock_wait_t * wait, int tmo);
void sock_wait_leave (sock_wait_t * wait, net_err_t err);

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
	net_err_t (*bind)(struct _sock_t* s, const struct x_sockaddr* addr, x_socklen_t len);
	net_err_t (*connect)(struct _sock_t* s, const struct x_sockaddr* addr, x_socklen_t len);
	net_err_t(*send)(struct _sock_t* s, const void* buf, size_t len, int flags, ssize_t * result_len);
	net_err_t(*recv)(struct _sock_t* s, void* buf, size_t len, int flags, ssize_t * result_len);
    net_err_t (*listen) (struct _sock_t *s, int backlog);
    net_err_t (*accept)(struct _sock_t *s, struct x_sockaddr* addr, x_socklen_t* len, struct _sock_t ** client);
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

	sock_wait_t * snd_wait;			// 发送等待
	sock_wait_t * rcv_wait;			// 接收等待
	sock_wait_t * conn_wait;	    // 连接等待

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
 * @brief 关闭sock消息
 */
typedef struct _sock_close_t {
    int dummy;
}sock_close_t;

/**
 * @brief 连接消息
 */
typedef struct _sock_conn_t {
    const struct x_sockaddr* addr;
    x_socklen_t len;
}sock_conn_t;

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
 * @brief 选项消息
 */
typedef struct _sock_opt_t {
    int level;
    int optname;
    const char * optval;
    int optlen;
}sock_opt_t;

/**
 * @brief bind请求
 */
typedef struct _sock_bind_t {
    const struct x_sockaddr* addr;
    x_socklen_t len;
}sock_bind_t;

/**
 * @brief listen参数
 * 
 */
typedef struct _sock_listen_t {
    int backlog;
}sock_listen_t;

/**
 * @brief accept消息
 */
typedef struct _sock_accept_t {
    struct x_sockaddr* addr;
    x_socklen_t * len;

    int client;                // 新接收的客户端消息
}sock_accept_t;

/**
 * @brief TCP API消息
 */
typedef struct _sock_req_t {
    sock_wait_t * wait;
    int wait_tmo;
    int sockfd;
    union {
        sock_create_t create;
        sock_data_t data;
        sock_opt_t opt;
        sock_close_t close;
        sock_conn_t conn;
        sock_bind_t bind;
        sock_listen_t listen;
        sock_accept_t accept;
    };
}sock_req_t;

net_err_t sock_init(sock_t* sock, int family, int protocol, const sock_ops_t * ops);
void sock_uninit (sock_t * sock);

net_err_t sock_create_req_in(func_msg_t* api_msg);
net_err_t sock_sendto_req_in (func_msg_t * api_msg);
net_err_t sock_recvfrom_req_in(func_msg_t * api_msg);
net_err_t sock_setsockopt_req_in(func_msg_t * api_msg);
net_err_t sock_close_req_in (func_msg_t* api_msg);
net_err_t sock_bind_req_in(func_msg_t * api_msg);
net_err_t sock_connect_req_in (func_msg_t* api_msg);
net_err_t sock_send_req_in (func_msg_t * api_msg);
net_err_t sock_recv_req_in(func_msg_t * api_msg);
net_err_t sock_listen_req_in(func_msg_t * api_msg);
net_err_t sock_accept_req_in(func_msg_t * api_msg);
net_err_t sock_destroy_req_in (func_msg_t* api_msg);

net_err_t sock_setopt(struct _sock_t* s,  int level, int optname, const char * optval, int optlen);
net_err_t sock_bind(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);
net_err_t sock_connect(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);
net_err_t sock_send (struct _sock_t * sock, const void* buf, size_t len, int flags, ssize_t * result_len);
net_err_t sock_recv (struct _sock_t * sock, void* buf, size_t len, int flags, ssize_t * result_len);

net_err_t socket_init(void);

net_err_t sock_wait (sock_t * sock, int type);
void sock_wakeup (sock_t * sock, int type, int err);

#endif // SOCK_H
