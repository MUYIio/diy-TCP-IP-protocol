#ifndef TCP_OUT_H
#define TCP_OUT_H

#include "tcp.h"

net_err_t tcp_send_reset (tcp_seg_t * seg);
net_err_t tcp_send_syn (tcp_t * tcp);

#endif
