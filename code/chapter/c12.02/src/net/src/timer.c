/**
 * @file timer.c
 * @author lishutong(lishutong@qq.com)
 * @brief 软定时器，用于ARP包，tcp超时重传等
 * @version 0.1
 * @date 2022-10-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string.h>
#include "timer.h"
#include "dbg.h"
#include "net_cfg.h"

static nlist_t timer_list;           // 当前软定时器列表

/**
 * @brief 添加定时器
 */
net_err_t net_timer_add(net_timer_t * timer, const char * name, timer_proc_t proc, void * arg, int ms, int flags) {
    return NET_ERR_OK;
}

/**
 * @brief 定时处理模块初始化
 */
net_err_t net_timer_init(void) {
    dbg_info(DBG_MSG, "timer init");

    nlist_init(&timer_list);

    dbg_info(DBG_MSG, "init done.");
    return NET_ERR_OK;
}
