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

/**
 * @brief 取消定时器
 */
void net_timer_remove (net_timer_t * timer) {
    dbg_info(DBG_TIMER, "remove timer: %s", timer->name);

    // 遍历列表，找到timer
    nlist_node_t * node;
    nlist_for_each(node, &timer_list) {
        net_timer_t * curr = nlist_entry(node, net_timer_t, node);
        if (curr != timer) {
            continue;
        }

        // 如果有后继结点，只需调整后继结点的值
        nlist_node_t * next = nlist_node_next(node);
        if (next) {
            net_timer_t * next_timer = nlist_entry(next, net_timer_t, node);
            next_timer->curr += curr->curr;
        }

        // 移除结点后结束
        nlist_remove(&timer_list, node);
        break;
    }

    // 更新完成后，显示下列表，方便观察
    display_timer_list();
}

/**
 * @brief 定时事件处理
 * 该函数不会被周期性的调用，其前后两次调用的时间未知
 */
net_err_t net_timer_check_tmo(int diff_ms) {
    // 需要重载的定时器链表
    nlist_t wait_list;
    nlist_init(&wait_list);

    // 遍历列表，看看是否有超时事件
    nlist_node_t* node = nlist_first(&timer_list);
    while (node) {
        // 预先取下一个，因为后面有调整结点的插入，链接关系处理不同，有可能导致整个遍历死循环
        nlist_node_t* next = nlist_node_next(node);

        // 减掉当前过去的时间，如果定时还未到，就可以退出了
        net_timer_t* timer = nlist_entry(node, net_timer_t, node);
        dbg_info(DBG_TIMER, "timer: %s, diff: %d, curr: %d, reload: %d\n",
                    timer->name, diff_ms, timer->curr, timer->reload);
        if (timer->curr > diff_ms) {
            timer->curr -= diff_ms;
            break;
        }

        // diff_time的时间可能比当前定时的时间还要大
        // 这意味着，后续可能还存在很多tmo!=0的情况需要处理
        // 所以，这里将diff_time给减掉一块，使得后续循环时能正确计算
        // 当然此时diff_time也可能为0，但为0也可能需要继续搜索，因为后面的timer的c_tmo也可能为0
        diff_ms -= timer->curr;

        // diff_time >= c_tmo，即超时时间到，包含c_tmo=0的情况
        // 定时到达，设置tmo = 0，从这里移除插入到待处理列表
        timer->curr = 0;
        nlist_remove(&timer_list, &timer->node);
        nlist_insert_last(&wait_list, &timer->node);

        // 继续搜索下一结点
        node = next;
    }

    // 执行定时函数，如果定时器需要重载，则重新插入链表
    while ((node = nlist_remove_first(&wait_list)) != (nlist_node_t*)0) {
        net_timer_t* timer = nlist_entry(node, net_timer_t, node);

        // 执行调用
        timer->proc(timer, timer->arg);

        // 重载定时器，先加入到等待插入的链表，避免破解现有的遍历
        if (timer->flags & NET_TIMER_RELOAD) {
            timer->curr = timer->reload;
            insert_timer(timer);
        }
    }

    display_timer_list();
    return NET_ERR_OK;
}

/**
 * @brief 获取第一个定时器的超时时间,以毫秒为单位
 */
int net_timer_first_tmo(void) {
    nlist_node_t* node = nlist_first(&timer_list);
    if (node) {
        net_timer_t* timer = nlist_entry(node, net_timer_t, node);
        return timer->curr;
    }

    return 0;
}
