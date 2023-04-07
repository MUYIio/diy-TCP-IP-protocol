#ifndef UDP_H
#define UDP_H

#include "sock.h"

typedef struct _udp_t {
    sock_t base;

    nlist_t recv_list;
    sock_wait_t recv_wait;
}udp_t;

net_err_t udp_init (void);
sock_t * udp_create (int family, int protocol);

#endif 