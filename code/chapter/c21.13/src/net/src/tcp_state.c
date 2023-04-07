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

/**
 * @brief 关闭状态下时来了一个数据报
 * 当输入的报文没有tcp处理时，或者tcp被关闭后，或者刚刚创建时
 * 此时，任何输入的报文都不应当被处理，直接给对方发送rst报文强制其复位
 */
net_err_t tcp_closed_in(tcp_t *tcp, tcp_seg_t *seg) {
    return NET_ERR_OK;
}

/**
 * @brief SYN已经发送时，收到的包处理
 * 正常的包：SYN+ACK包(自己先打开)，或者只收到SYN包（对方同时打开）
 * 其它包：丢弃或者发送RST后丢弃
 * 在该连接阶段，不支持附带发送任何数据，也不接收任何数据
 *
 * 触发方式：由用户调用connect后进入该状态
 */
net_err_t tcp_syn_sent_in(tcp_t *tcp, tcp_seg_t *seg) {
    return NET_ERR_OK;
}

/**
 * @brief 已经建立连接时的状态处理
 * 这个状态下，双方已经建立连接，可以互相发送数据
 */
net_err_t tcp_established_in(tcp_t *tcp, tcp_seg_t *seg) {
    return NET_ERR_OK;
}

/**
 * @brief 被动关闭时状态处理
 * 此时，连接已经被远程主动发起关闭，即远端已经发送过FIN通知我们他的数据发完了
 * 这个时候我们可以继续发送数据给对方，但是不需要再接收数据了
 */
net_err_t tcp_close_wait_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}

/**
 * @brief last ack状态下的输入处理
 * 此时，对方先发起了关闭；我们确认后也发送了FIN。该状态下，当对方的ACK到来时，
 * 即确认完成了整个连接的关闭。在这个过程中不再接收任何数据
 */
net_err_t tcp_last_ack_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}

/**
 * @brief TCP FIN-wait 1状态处理
 * 在该状态下，对方未关闭，但是我们自己先主动关闭，表明自己不需要再发送数据。但是仍然可以接收数据
 */
net_err_t tcp_fin_wait_1_in(tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}

/**
 * @brief TCP FIN-wait 2状态处理
 * 在该状态下，本机已经关闭了发送；但还可以接收远端的数据
 */
net_err_t tcp_fin_wait_2_in(tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}

/**
 * @brief TCP closing状态下的处理
 * 在此状态下，已经收到了对方关闭的请求并做了响应，等待对方对自己的关闭请求进行响应
 * 此时不接受任何数据处理，收到ACK后，立即进入time-wait状态
 */
net_err_t tcp_closing_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}

/**
 * @brief 进入time wait时的状态处理
 * 在该状态下，连接已经被关闭。tcp只是用来重传ack使用，即对对方的FIN进行回应
 */
net_err_t tcp_time_wait_in (tcp_t * tcp, tcp_seg_t * seg) {
    return NET_ERR_OK;
}
