#ifndef SOCK_H
#define SOCK_H

#include "net_errr.h"

typedef struct _x_socket_t {
    enum {
        SOCKET_STATE_FREE,
        SOCKET_STATE_USED,
    }state;
}x_socket_t;

typedef struct _sock_create_t {
     int family;
     int type;
     int protocol;
}sock_create_t;

typedef struct _sock_req_t {
    int sockfd;
    union {
        sock_create_t create;
    };
   
}sock_req_t;

net_err_t socket_init (void);
net_err_t sock_create_req_in (struct _func_msg_t * msg);

#endif 