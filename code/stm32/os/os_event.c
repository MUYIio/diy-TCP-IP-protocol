#include "os_event.h"

/**
 * 初始化事件结构
 * @param event 事件结构
 * @param type 类型
 * @param flags 初始的标志等
 * @return 初始化结果
*/
os_err_t os_event_init (os_event_t * event, os_event_type_t type, int flags) {
    os_assert(event != OS_NULL);
    os_assert((type >= OS_EVENT_TYPE_INVALID) && (type < OS_EVENT_TYPE_MAX));

	event->type = type;
    event->flags = flags;

    event->first = event->last = OS_NULL;
    return OS_ERR_OK;
}

/**
 * 反初始化化事件结构
 * 当任务再次运行时，其得到的错误码为OS_ERR_REMOVE
 * 
 * @param event 事件结构
 * @return 初始化结果
 */
os_err_t os_event_uninit (os_event_t * event) {
    os_assert(event != OS_NULL);

    os_event_clear(event, OS_NULL, OS_ERR_REMOVE);
    return OS_ERR_OK;
}

/**
 * 将wait插入到event中等待从队列的末尾
 * @param event 被插入的事件结构
 * @param 等待队列
 */
static void os_event_add_wait (os_event_t * event, os_wait_t * wait) {
    os_assert(event != OS_NULL);
    os_assert(wait != OS_NULL);

    if (event->first == OS_NULL) {
        event->first = wait;
        event->last = wait;
    } else {
        event->last->next = wait;
        event->last = wait;
    }
}

/**
 * 移除event头部的移除
 * @param event 被插入的事件结构
 * @param 等待队列
 */
static os_wait_t * os_event_remove_first (os_event_t * event) {
    os_assert(event != OS_NULL);
    
    if (event->first == OS_NULL) {
        return OS_NULL;
    } else {
        os_wait_t * wait = event->first;
        event->first = wait->next;
        if (event->last == wait) {
            event->last = OS_NULL;
        }
		return wait;
	}
}

/**
 * 移除event中的指定等待事件
 * @param event 被插入的事件结构
 */
static os_wait_t * os_event_remove_wait(os_event_t * event, os_wait_t * wait) {
    os_assert(event != OS_NULL);
    os_assert(wait != OS_NULL);

    os_wait_t * pre = OS_NULL;
    os_wait_t * curr = event->first;
    while (curr) {
        if (curr == wait) {
            if (pre) {
                pre->next = curr->next;
            }

            if (event->first == curr) {
                event->first = curr->next;
            }
            if (event->last == curr) {
                event->last = pre;
            }

            curr->next = OS_NULL;
            return curr;
        }
        curr = curr->next;
    }

    return OS_NULL;
}

/**
 * 让当前任务在事件控制块上等待事件发生
 * @param event 等待的事件
 * @param reason 事件原因
 * @param ms 等待的事件，如果为0将一直超时
 * @return 等待结果
 */
os_err_t os_event_wait (os_event_t * event, void * reason, int ms) {
    os_assert(event != OS_NULL);
    os_assert(ms >= 0);
    
    os_wait_t wait;
    os_task_t * self = os_task_self();
    
    // 初始化等待结点
    wait.next = (os_wait_t *)0;
    wait.event = event;
    wait.task = self;
    wait.reason = reason;
    wait.err = OS_ERR_OK;

    // 设置任务等待状态
    self->wait = &wait;

    // 从就绪队列中移除，进入延时队列和事件等待队列
    os_protect_t protect = os_enter_protect();
    os_sched_set_unready(self);
    os_task_set_delay(self, ms);
    os_event_add_wait(event, &wait);
    os_leave_protect(protect);

    // 切换，进入等待过程
    os_sched_run();
    return wait.err;
}

/**
 * 唤醒事件控制块中的首个等待的任务
 * @param 等待的事件
 * @param 等待原因
 * @param err 唤醒原因
 * @return 唤醒的任务
*/
os_task_t * os_event_wakeup (os_event_t * event) {
    os_assert(event != OS_NULL);

    // 从等待队列中移除
    os_wait_t * wait = os_event_remove_first(event);
    if (wait == OS_NULL) {
        return OS_NULL;
    }

    // 如果有延时，从延时队列中移除
    os_task_t * task = wait->task;
    if (task->flags & OS_TASK_FLAG_DELAY) {
        os_task_set_wakeup(task);
    }

    // 进入就绪队列
    os_sched_set_ready(task);
    return task;
}

/**
 * 唤醒事件控制块中的指定的任务
 * @param 等待的事件
 * @param 等待原因
 * @param err 唤醒原因
 * @return 唤醒的任务
*/
void os_event_wakeup_task (os_event_t * event, os_task_t * task, void * reason, os_err_t err) {
    os_assert(event != OS_NULL);
    os_assert(task != OS_NULL);

     // 从等待队列中移除
    os_wait_t * wait = os_event_remove_wait(event, task->wait);
    os_assert(wait != OS_NULL);

    // 填充结果
    wait->err = err;
    wait->reason = reason;

    // 如果有延时，从延时队列中移除
    if (task->flags & OS_TASK_FLAG_DELAY) {
        os_task_set_wakeup(task);
    }
    task->wait = OS_NULL;

    // 进入就绪队列
    os_sched_set_ready(task);
}

/**
 * 清除所有等待中的任务，将事件发送给所有任务
 * @param event 事件控制块
 * @param reason 清除原因
 * @param err 清除原因
 * @return 有多少个任务被清除掉
*/
int os_event_clear (os_event_t * event, void * reason, os_err_t err) {
    os_assert(event != OS_NULL);

    int cnt = 0;
    while (event->first) {        
        // 如果有延时，从延时队列中移除
        os_task_t * task = event->first->task;
        if (task->delay_tick) {
            os_task_set_wakeup(task);
        }
        task->wait->err = err;
        task->wait = OS_NULL;

        // 进入就绪队列
        os_sched_set_ready(task);

        // 继续下一个等待结构
        event->first = event->first->next;
        cnt++;
    }
    return  cnt;
}

/**
 * 获取等待队列中任务的数量
 * @param event 事件控制块
 */
int os_event_wait_cnt (os_event_t * event) {
    os_assert(event != OS_NULL);

    int cnt = 0;
    for (os_wait_t * wait = event->first; wait; wait = wait->next) {
        cnt++;
    }
    return cnt;
}  



