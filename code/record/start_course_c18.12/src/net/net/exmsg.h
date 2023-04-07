#ifndef EXMSG_H
#define EXMSG_H

#include "net_errr.h"
#include "nlist.h"
#include "netif.h"

struct _func_msg_t;
typedef net_err_t (*exmsg_func_t) (struct _func_msg_t * msg);


typedef struct _msg_netif_t {
    netif_t * netif;
}msg_netif_t;

typedef struct _func_msg_t {
    sys_thread_t thread;

    exmsg_func_t func;
    void * param;
    net_err_t err;

    sys_sem_t wait_sem;
}func_msg_t;

typedef struct _exmsg_t {
    nlist_node_t node;
    
    enum {
        NET_EXMSG_NETIF_IN,
        NET_EXMSG_FUN,
    }type;

    union {
        msg_netif_t netif;
        func_msg_t * func;
    };
}exmsg_t;

net_err_t exmsg_init (void);
net_err_t exmsg_start (void);
net_err_t exmsg_netif_in(netif_t * netif);
net_err_t exmsg_func_exec (exmsg_func_t func, void * param);

#endif 
