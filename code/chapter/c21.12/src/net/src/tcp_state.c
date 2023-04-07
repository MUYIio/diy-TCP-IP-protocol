/**
 * @file tcp_state.c
 * @author lishutong
 * @brief TCP输入处理, 对包进行各种检查，并处理状态机相关事项
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022
 * 
 * 处理TCP的各种状态转换过程
 */
#include "dbg.h"
#include "mblock.h"
#include "nlist.h"
#include "tcp_out.h"
#include "tcp_state.h"
#include "tools.h"
#include "socket.h"
#include "protocol.h"
#include "tcp_in.h"
#include "tcp_out.h"
#include "tcp.h"

/**
 * @brief 获取TCP状态名称 
 */
const char * tcp_state_name (tcp_state_t state) {
    static const char * state_name[] = {
        [TCP_STATE_FREE] = "FREE",
        [TCP_STATE_CLOSED] = "CLOSED",
        [TCP_STATE_LISTEN] = "LISTEN",
        [TCP_STATE_SYN_SENT] = "SYN_SENT",
        [TCP_STATE_SYN_RECVD] = "SYN_RCVD",
        [TCP_STATE_ESTABLISHED] = "ESTABLISHED",
        [TCP_STATE_FIN_WAIT_1] = "FIN_WAIT_1",
        [TCP_STATE_FIN_WAIT_2] = "FIN_WAIT_2",
        [TCP_STATE_CLOSING] = "CLOSING",
        [TCP_STATE_TIME_WAIT] = "TIME_WAIT",
        [TCP_STATE_CLOSE_WAIT] = "CLOSE_WAIT",
        [TCP_STATE_LAST_ACK] = "LAST_ACK",

        [TCP_STATE_MAX] = "UNKNOWN",
    };

    if (state >= TCP_STATE_MAX) {
        state = TCP_STATE_MAX;
    }
    return state_name[state];
}

/**
 * @brief 设置TCP状态并显示
 */
void tcp_set_state (tcp_t * tcp, tcp_state_t state) {
    tcp->state = state;

    tcp_show_info("tcp set state", tcp);
}
