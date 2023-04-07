/**
 * @file tcp_out.h
 * @author lishutong(527676163@qq.com)
 * @brief tcp输出模块
 * @version 0.1
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef TCP_OUT_H
#define TCP_OUT_H

#include "tcp.h"

/**
 * @brief TCP事件
 */
typedef enum _tcp_oevent_t {
    TCP_OEVENT_SEND,            // 新数据的发送事件
    TCP_OEVENT_XMIT,            // 继续缓存中的数据事件
    TCP_OEVENT_TMO,
}tcp_oevent_t;

// 以下是一些基本的不含数据的发送，实现上简单一些
net_err_t tcp_send_reset(tcp_seg_t * seg);
net_err_t tcp_send_reset_for_tcp(tcp_t* tcp);
net_err_t tcp_send_ack(tcp_t* tcp, tcp_seg_t * seg);
net_err_t tcp_send_syn(tcp_t* tcp);
net_err_t tcp_send_fin (tcp_t* tcp);
net_err_t tcp_transmit(tcp_t * tcp);
net_err_t tcp_retransmit(tcp_t* tcp, int no_newdata);
net_err_t tcp_send_win_update (tcp_t * tcp);
net_err_t tcp_send_keepalive(tcp_t* tcp);

net_err_t tcp_ack_process (tcp_t * tcp, tcp_seg_t * seg);
net_err_t tcp_begin_persist (tcp_t * tcp);
void tcp_end_persist (tcp_t * tcp);
void tcp_cal_rto (tcp_t * tcp);
void tcp_cal_snd_win (tcp_t * tcp, tcp_seg_t * seg);

int tcp_write_sndbuf(tcp_t * tcp, const uint8_t * buf, int len);
void tcp_out_event (tcp_t * tcp, tcp_oevent_t event);
void tcp_set_ostate (tcp_t * tcp, tcp_ostate_t state);
const char * tcp_ostate_name (tcp_t * tcp);

#endif // !TCP_OUT_H
