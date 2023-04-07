#include "tcp_state.h"

const char * tcp_state_name (tcp_state_t state) {
    static const char * state_name[] = {
        [TCP_STATE_CLOSED] = "CLOSED",
        [TCP_STATE_LISTEN] = "LISTEN",
        [TCP_STATE_SYN_SENT] = "SYN_SENT",
        [TCP_STATE_SYN_RECVD] = "SYN_RECVD",
        [TCP_STATE_ESTABLISHED] = "ESTABLISHED",
        [TCP_STATE_FIN_WAIT_1] = "FIN_WAIT_1",
        [TCP_STATE_FIN_WAIT_2] = "FIN_WAIT_2",
        [TCP_STATE_CLOSING] = "CLOSING",
        [TCP_STATE_TIME_WAIT] = "TIME_WAIT",
        [TCP_STATE_CLOSE_WAIT] = "CLOSE_WAIT",
        [TCP_STATE_LAST_ACK] = "LAST_ACK",

        [TCP_STATE_MAX] = "UNKNOWN",
    };

    if (state > TCP_STATE_MAX) {
        state = TCP_STATE_MAX;
    }

    return state_name[state];
}

void tcp_set_state (tcp_t * tcp, tcp_state_t state) {
    tcp->state = state;
    tcp_show_info("tcp set state", tcp);
}

net_err_t tcp_closed_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}

net_err_t tcp_listen_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_syn_sent_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_syn_recvd_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_established_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_fin_wait_1_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_fin_wait_2_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_closing_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_time_wait_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_close_wait_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
net_err_t tcp_last_ack_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}