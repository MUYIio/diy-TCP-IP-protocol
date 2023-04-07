/**
 * @file tcp.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "tcp.h"
#include "tools.h"
#include "protocol.h"
#include "ipv4.h"
#include "mblock.h"
#include "dbg.h"
#include "exmsg.h"
#include "socket.h"
#include "timer.h"

static tcp_t tcp_tbl[TCP_MAX_NR];           // tcp控制块数组
static mblock_t tcp_mblock;                 // 空闲TCP控制块链表
static nlist_t tcp_list;                     // 已创建的控制块链表

/**
 * TCP模块初始化
 */
net_err_t tcp_init(void) {
    dbg_info(DBG_TCP, "tcp init.");

    mblock_init(&tcp_mblock, tcp_tbl, sizeof(tcp_t), TCP_MAX_NR, NLOCKER_NONE);
    nlist_init(&tcp_list);

    dbg_info(DBG_TCP, "init done.");
    return NET_ERR_OK;
}
