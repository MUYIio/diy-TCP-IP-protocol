#ifndef SOCK_H
#define SOCK_H

#include "net_errr.h"

typedef struct _x_socket_t {
    enum {
        SOCKET_STATE_FREE,
        SOCKET_STATE_USED,
    }state;
}x_socket_t;

net_err_t socket_init (void);

#endif 