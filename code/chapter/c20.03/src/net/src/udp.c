/**
 * @file udp.c
 * @author lishutong(527676163@qq.com)
 * @brief UDP协议层处理，以及sock接口的实现
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 */
#include <string.h>
#include "dbg.h"
#include "udp.h"
#include "mblock.h"

static udp_t udp_tbl[UDP_MAX_NR];           // UDP控制块数组
static mblock_t udp_mblock;                 // 空闲UDP控制块链表
static nlist_t udp_list;                     // 已绑定的控制块链表

/**
 * @brief UDP模块初始化
 */
net_err_t udp_init(void) {
    dbg_info(DBG_UDP, "udp init.");

    // 应用端可能有多任务访问，因此需要锁
    mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), UDP_MAX_NR, NLOCKER_NONE);
    nlist_init(&udp_list);

    dbg_info(DBG_UDP, "init done.");
    return NET_ERR_OK;
}
