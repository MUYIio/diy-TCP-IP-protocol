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

// 以下是一些基本的不含数据的发送，实现上简单一些
net_err_t tcp_send_reset(tcp_seg_t * seg);
net_err_t tcp_send_ack(tcp_t* tcp, tcp_seg_t * seg);
net_err_t tcp_send_syn(tcp_t* tcp);
net_err_t tcp_ack_process (tcp_t * tcp, tcp_seg_t * seg);

#endif // !TCP_OUT_H
