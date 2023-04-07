﻿/**
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
 * @brief 将结点按超时时间从小到达插入进链表中
 */
static void insert_timer(net_timer_t * insert) {
    nlist_node_t* node;
    nlist_node_t *pre = (nlist_node_t *)0;

    nlist_for_each(node, &timer_list) {
        net_timer_t * curr = nlist_entry(node, net_timer_t, node);

        // 待插入的结点超时比当前结点超时大，应当继续往后寻找
        // 因此，先将自己的时间减一下，然后继续往下遍历
        if (insert->curr > curr->curr) {
            insert->curr -= curr->curr;
        } else if (insert->curr == curr->curr) {
            // 相等，插入到其之后，超时调整为0，即超时相等
            insert->curr = 0;
            nlist_insert_after(&timer_list, node, &insert->node);
            return;
        } else {
            // 比当前超时短，插入到当前之前，那么当前的超时时间要减一下
            curr->curr -= insert->curr;
            if (pre) {
                nlist_insert_after(&timer_list, pre, &insert->node);
            } else {
                nlist_insert_first(&timer_list, &insert->node);
            }
            return;
        }
        pre = node;
    }

    // 找不到合适的位置，即超时比所有的都长，插入到最后
    nlist_insert_last(&timer_list, &insert->node);
}

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
    insert_timer(timer);

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
