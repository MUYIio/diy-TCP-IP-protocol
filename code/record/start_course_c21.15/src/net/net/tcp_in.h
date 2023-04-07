#ifndef TCP_IN_H
#define TCP_IN_H

#include "tcp.h"
#include "pktbuf.h"
#include "ipaddr.h"

net_err_t tcp_in (pktbuf_t * buf, ipaddr_t * src_ip, ipaddr_t * dest_ip);


#endif
