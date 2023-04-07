/**
 * @file timer.h
 * @author lishutong(lishutong@qq.com)
 * @brief 软定时器，用于ARP包，tcp超时重传等
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef TIMER_H
#define TIMER_H

#include "net_cfg.h"
#include "net_err.h"
#include "nlist.h"

#define NET_TIMER_RELOAD        (1 << 0)       // 自动重载

/**
 * @brief 定时处理函数
 */
struct _net_timer_t;       // 前置声明
typedef void (*timer_proc_t)(struct _net_timer_t* timer, void * arg);

/**
 * @brief 定时器结构
 */
typedef struct _net_timer_t {
    char name[TIMER_NAME_SIZE];     // 定时器名称
    int flags;                          // 是否自动重载

    int curr;                           // 当前超时值，以毫秒计
    int reload;                         // 重载的定时值，以毫秒计

    timer_proc_t proc;                  // 定时处理函数
    void * arg;                         // 定义参数
    nlist_node_t node;                   // 链接接点
}net_timer_t;

net_err_t net_timer_init(void);
net_err_t net_timer_add(net_timer_t * timer, const char * name, timer_proc_t proc, void * arg, int ms, int flags);
void net_timer_remove (net_timer_t * timer);
net_err_t net_timer_check_tmo(int diff_ms);
int net_timer_first_tmo(void);

#endif // TIMER_H
