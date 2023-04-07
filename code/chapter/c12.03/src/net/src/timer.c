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
 * @brief 显示定时器列表信息
 */
#if DBG_DISP_ENABLED(DBG_TIMER)
static void display_timer_list(void) {
    plat_printf("--------------- timer list ---------------\n");

    nlist_node_t* node;
    int index = 0;
    nlist_for_each(node, &timer_list) {
        net_timer_t* timer = nlist_entry(node, net_timer_t, node);

        plat_printf("%d: %s, period = %d, curr: %d ms, reload: %d ms\n",
            index++, timer->name,
            timer->flags & NET_TIMER_RELOAD ? 1 : 0,
            timer->curr, timer->reload);
    }
    plat_printf("---------------- timer list end ------------\n");
}
#else
#define display_timer_list()
#endif

/**
 * @brief 添加定时器
 */
net_err_t net_timer_add(net_timer_t * timer, const char * name, timer_proc_t proc, void * arg, int ms, int flags) {
    dbg_info(DBG_TIMER, "insert timer: %s", name);

    plat_strncpy(timer->name, name, TIMER_NAME_SIZE);
    timer->name[TIMER_NAME_SIZE - 1] = '\0';
    timer->reload = ms;
    timer->curr = timer->reload;
    timer->proc = proc;
    timer->arg = arg;
    timer->flags = flags;

    // 插入到定时器链表中
    nlist_insert_last(&timer_list, &timer->node);
    // insert_timer(timer);

    display_timer_list();
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
