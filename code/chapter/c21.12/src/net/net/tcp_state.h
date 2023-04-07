/**
 * @file tcp_state.h
 * @author lishutong(527676163@qq.com)
 * @brief tcp状态处理
 * @version 0.1
 * @date 2022-11-17
 * 
 * @copyright Copyright (c) 2022
 * 
 * 处理TCP的各种状态转换过程
 */
#ifndef TCP_STATE_H
#define TCP_STATE_H

#include "tcp.h"

const char * tcp_state_name (tcp_state_t state);
void tcp_set_state (tcp_t * tcp, tcp_state_t state);

#endif // TCP_STATE_H
