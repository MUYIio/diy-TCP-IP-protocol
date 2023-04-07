#ifndef SOCK_H
#define SOCK_H

#include "net_errr.h"
#include "nlist.h"
#include "sys.h"
#include "ipaddr.h"

struct _sock_t;
typedef int x_socklen_t;
struct x_sockaddr;
struct _sock_req_t;

#define SOCK_WAIT_READ      (1 << 0)
#define SOCK_WAIT_WRITE     (1 << 1)
#define SOCK_WAIT_CONN      (1 << 2)
#define SOCK_WAIT_ALL       (SOCK_WAIT_READ | SOCK_WAIT_WRITE | SOCK_WAIT_CONN)

typedef struct _sock_wait_t {
    sys_sem_t sem;
    net_err_t err;
    int waiting;
}sock_wait_t;

net_err_t sock_wait_init (sock_wait_t * wait);
void sock_wait_destory (sock_wait_t * wait);
void sock_wait_add (sock_wait_t * wait, int tmo, struct _sock_req_t * req);
net_err_t sock_wait_enter (sock_wait_t * wait, int tmo);
void sock_wait_leave (sock_wait_t * wait, net_err_t err);

typedef struct _sock_ops_t {
    net_err_t (*close) (struct _sock_t * s);
    net_err_t (*sendto)(struct _sock_t * s, const void * buf, size_t len, int flags, 
                const struct x_sockaddr * dest, x_socklen_t dest_len, ssize_t * result_len);
    net_err_t (*recvfrom)(struct _sock_t * s, void * buf, size_t len, int flags, 
                const struct x_sockaddr * src, x_socklen_t * src_len, ssize_t * result_len);
    
    net_err_t (*setopt)(struct _sock_t * s, int level, int optname, const char * optval, int optlen);
    void (*destory) (struct _sock_t * s);
}sock_ops_t;

typedef struct _sock_t {
    uint16_t local_port;
    ipaddr_t local_ip;
    ipaddr_t remote_ip;
    uint16_t remote_port;

    const sock_ops_t * ops;
    int family;
    int protocol;
    int err;
    int rcv_tmo;
    int snd_tmo;

    sock_wait_t * rcv_wait;
    sock_wait_t * snd_wait;
    sock_wait_t * conn_wait;

    nlist_node_t node;
}sock_t;

typedef struct _x_socket_t {
    enum {
        SOCKET_STATE_FREE,
        SOCKET_STATE_USED,
    }state;

    sock_t * sock;
}x_socket_t;

typedef struct _sock_create_t {
     int family;
     int type;
     int protocol;
}sock_create_t;

typedef struct _sock_data_t {
    uint8_t * buf;
    size_t len;
    int flags;
    struct x_sockaddr * addr;
    x_socklen_t * addr_len; 
    ssize_t comp_len;
}sock_data_t;

typedef struct _sock_opt_t {
    int level;
    int optname;
    const char * optval;
    int len;
}sock_opt_t;

typedef struct _sock_req_t {
    sock_wait_t * wait;
    int wait_tmo;

    int sockfd;
    union {
        sock_create_t create;
        sock_data_t data;
        sock_opt_t opt;
    };
   
}sock_req_t;

net_err_t socket_init (void);
net_err_t sock_create_req_in (struct _func_msg_t * msg);
net_err_t sock_sendto_req_in (struct _func_msg_t * msg);
net_err_t sock_recvfrom_req_in (struct _func_msg_t * msg);
net_err_t sock_setsockopt_req_in (struct _func_msg_t * msg);
net_err_t sock_setopt (struct _sock_t * s, int level, int optname, const char * optval, int optlen);


net_err_t sock_init (sock_t * sock, int family, int protocol, const sock_ops_t * ops);
void sock_uninit(sock_t * sock);

#endif 