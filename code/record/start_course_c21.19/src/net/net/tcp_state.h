#ifndef TCP_STATE_H
#define TCP_STATE_H

#include "tcp.h"

typedef net_err_t (*tcp_proc_t)(tcp_t * tcp, tcp_seg_t * seg);

net_err_t tcp_closed_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_listen_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_syn_sent_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_syn_recvd_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_established_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_fin_wait_1_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_fin_wait_2_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_closing_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_time_wait_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_close_wait_in (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_last_ack_in (tcp_t * tcp, tcp_seg_t * seg);


const char * tcp_state_name (tcp_state_t state);
void tcp_set_state (tcp_t * tcp, tcp_state_t state);

#endif