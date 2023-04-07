#include "os_core.h"
#include "os_mem.h"
#include "os_timer.h"

typedef struct _os_timer_core_t {
    os_list_t timer_stlist;             // 软定时器列表 
    os_list_t timer_hdlist;             // 硬定时器列表
}os_timer_core_t;

static os_timer_core_t timer_core;

/**
 * 初始化定时器
 * @param timer 定时器
 * @param delay_ms 延时启动的毫秒数
 * @param tmo_ms 超时时间间隔
 * @param func 定时回调函数
 * @param arg 回调函数的参数
*/
static os_err_t timer_init (os_timer_t * timer, int delay_ms, int tmo_ms, 
                os_timer_func_t func, void * arg, int flags) {
    timer->next = OS_NULL;
    timer->tmo_ms = tmo_ms >= 0 ? tmo_ms : 0;
    timer->delay_ms = delay_ms > 0 ? delay_ms : tmo_ms;
    timer->func = func;
    timer->arg = arg;
    timer->flags = flags;
    timer->state = OS_TIMER_INITED;
    return OS_ERR_OK;
}

/**
 * 初始化一个定时器
 * @param timer 定时器
 * @param delay_ms 延时启动的毫秒数
 * @param tmo_ms 超时时间间隔
 * @param func 定时回调函数
 * @param arg 回调函数的参数
 * @param flags 相关标志
 */
os_err_t os_timer_init (os_timer_t * timer, int delay_ms, int tmo_ms, os_timer_func_t func, void * arg, int flags) {
    os_param_failed(timer == OS_NULL, OS_ERR_PARAM);
    os_param_failed((delay_ms <= 0) && (tmo_ms <= 0), OS_ERR_PARAM);  // 允许其中一个为0
    os_param_failed(func == OS_NULL, OS_ERR_PARAM);

    return timer_init(timer, delay_ms, tmo_ms, func, arg, flags);
}

/**
 * 取消定时器的初始化
 * @param timer 定时器
*/
os_err_t os_timer_uninit(os_timer_t * timer) {
    os_param_failed(timer == OS_NULL, OS_ERR_PARAM);
    return OS_ERR_OK;
}

/**
 * 创建一个定时器
 * @param timer 定时器
 * @param delay_ms 延时启动的毫秒数
 * @param tmo_ms 超时时间间隔
 * @param func 定时回调函数
 * @param arg 回调函数的参数
 * @param flags 相关标志
 * @return 创建的定时器
 */
os_timer_t * os_timer_create (int delay_ms, int tmo_ms, os_timer_func_t func, void * arg, int flags) {
    os_param_failed((delay_ms <= 0) && (tmo_ms <= 0), OS_NULL);  // 允许其中一个为0
    os_param_failed(func == OS_NULL, OS_NULL);

    // 分配内存
    os_timer_t * timer = os_mem_alloc(sizeof(os_timer_t));
    if (timer == OS_NULL) {
        os_dbg("error: alloc sem failed");
        return OS_NULL;
    }

    // 初始化信号量结构
    os_err_t err = timer_init(timer, delay_ms, tmo_ms, func, arg, flags);
    if (err < 0) {
        os_dbg("error: init sem failed.");
        os_mem_free(timer);
        return OS_NULL;
    }

    return timer;
}

/**
 * 销毁一个定时器
 * @param 需要释放的信号量
 */
os_err_t os_timer_free (os_timer_t * timer) {
    os_param_failed(timer == OS_NULL, OS_ERR_PARAM);

    os_timer_uninit(timer);
    os_mem_free(timer);
    return OS_ERR_OK;
}

/**
 * 获取定时器还有多长时间超时
 * @param timer 查询的定时器
*/
int os_timer_left (os_timer_t * timer) {
    os_param_failed(timer == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int ms = timer->tmo_ms;
    os_leave_protect(protect);
    return ms;
}

/**
 * 启动定时器
 * @param timer 待启动的定时器
 */
os_err_t os_timer_start (os_timer_t * timer) {
    os_param_failed(timer == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    if ((timer->state != OS_TIMER_INITED) && (timer->state != OS_TIMER_STOPPED)) {
        os_leave_protect(protect);
        return OS_ERR_STATE;
    }

    if (timer->flags & OS_TIMER_HD) {
        //os_list_append(&timer_core.timer_hdlist, &timer->linkNode);
    } else {
        //os_list_append(&tTimerSoftList, &timer->linkNode);
    }
    timer->state = OS_TIMER_STARTED;

    os_leave_protect(protect);
    return OS_ERR_OK;
}

/**
 * 终止定时器
 * @param timer 待终止的定时器
 */
os_err_t os_timer_stop (os_timer_t * timer) {
    os_param_failed(timer == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    if (timer->state != OS_TIMER_INITED) {
        os_leave_protect(protect);
        return OS_ERR_STATE;
    }

    if (timer->flags & OS_TIMER_HD) {
        //os_list_remove(&tTimerHardList, &timer->linkNode);
    } else {
        //os_list_remove(&tTimerSoftList, &timer->linkNode);
    }
    timer->state = OS_TIMER_STOPPED;
    
    os_leave_protect(protect);
    return OS_ERR_OK;
}

/**
 * 遍历指定的定时器列表，调用各个定时器处理函数
 */
void os_timer_tick (void) {
#if 0
    tNode * node;
    
    // 检查所有任务的delayTicks数，如果不0的话，减1。
    for (node = timerList->headNode.nextNode; node != &(timerList->headNode); node = node->nextNode)
    {
        tTimer * timer = tNodeParent(node, tTimer, linkNode);

        // 如果延时已到，则调用定时器处理函数
        if ((timer->delayTicks == 0) || (--timer->delayTicks == 0))
        {
            // 切换为正在运行状态
            timer->state = tTimerRunning;

            // 调用定时器处理函数
            timer->timerFunc(timer->arg);

            // 切换为已经启动状态
            timer->state = tTimerStarted;

            if (timer->durationTicks > 0)
            {
                // 如果是周期性的，则重复延时计数值
                timer->delayTicks = timer->durationTicks;
            }
            else
            {
                // 否则，是一次性计数器，中止定时器
                os_list_remove(timerList, &timer->linkNode);
                timer->state = tTimerStopped;
            }
        }
    }
#endif
}

/**
 * 初始化定时器模块
 */
void os_timer_core_init (void) {

}
