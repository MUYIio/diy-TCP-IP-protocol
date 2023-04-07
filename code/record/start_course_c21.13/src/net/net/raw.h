#ifndef RAW_H
#define RAW_H

#include "sock.h"
#include "pktbuf.h"
#include "nlist.h"

typedef struct _raw_t {
    sock_t base;

    nlist_t recv_list;
    sock_wait_t recv_wait;
}raw_t;


net_err_t raw_init (void);
sock_t * raw_create (int family, int protocol);
net_err_t raw_in (pktbuf_t * pktbuf);

#endif
