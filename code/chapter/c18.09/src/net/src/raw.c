/**
 * @file raw.c
 * @author lishutong(527676163@qq.com)
 * @brief 原始套接字
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 * 
 * 给应用层使用的IP控制块, 用于收发IP层的数据报，可以实现例如ping等功能
 */
#include "raw.h"
#include "mblock.h"
#include "dbg.h"
#include "sock.h"
#include "socket.h"

static raw_t raw_tbl[RAW_MAX_NR];           // raw控制块数组
static mblock_t raw_mblock;                 // 空闲raw控制块链表
static nlist_t raw_list;                     // 已绑定的控制块链表

/**
 * @brief RAW模块初始化
 */
net_err_t raw_init(void) {
    dbg_info(DBG_RAW, "raw init.");

    mblock_init(&raw_mblock, raw_tbl, sizeof(raw_t), RAW_MAX_NR, NLOCKER_NONE);
    nlist_init(&raw_list);

    dbg_info(DBG_RAW, "init done.");
    return NET_ERR_OK;
}
