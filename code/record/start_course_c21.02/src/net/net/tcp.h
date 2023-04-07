#ifndef TCP_H
#define TCP_H

#include "sock.h"

typedef struct _tcp_t {
    sock_t base;

}tcp_t;

net_err_t tcp_init (void);
sock_t * tcp_create (int family, int protocol);

#endif
