#include "os_mutex.h"
#include "os_mem.h"
#include "os_list.h"

#if OS_MUTEX_ENABLE

os_err_t mutex_init (os_mutex_t * mutex, int flags) {
    // 初始化等待结构
    os_err_t err = os_event_init(&mutex->event, OS_EVENT_TYPE_MUTEX, flags);
    if (err < 0) {
        os_dbg("create mutex event failed: %s.", os_err_str(err));
        return err;
    }

    mutex->lock_cnt = 0;
    mutex->owner = (os_task_t *)0;
    mutex->raw_prio = -1;
    return OS_ERR_OK;
}

/**
 * 初始化互斥信号量
 * @param 待初始化的互斥信号量
 * @return 初始化结果
*/
os_err_t os_mutex_init (os_mutex_t * mutex) {
    os_param_failed(mutex == OS_NULL, OS_ERR_PARAM);

    return mutex_init(mutex, 0);
}

/**
 * 取消互斥信号量的初始化
 * @param mutex 互斥信号量
*/
os_err_t os_mutex_uninit (os_mutex_t * mutex) {       
    os_param_failed(mutex == OS_NULL, OS_ERR_PARAM);

    // 处理优先级翻转的情况
    os_task_t * owner = mutex->owner;
    if (owner && (mutex->raw_prio != owner->prio)) {
        os_dbg("inherit prio: reset from %d to %d.\n", owner->prio, mutex->raw_prio);

        // 有发生优先级继承，恢复拥有者的优先级
        if ((owner->flags & OS_TASK_FLAG_DELAY) || owner->wait) {
            // 正在等某些，不改优先级
            owner->prio = mutex->raw_prio;
        } else {
            // 就绪状态，调整其所在队列
            os_sched_set_unready(owner);
            owner->prio = mutex->raw_prio;
            os_sched_set_ready(owner);
        }
    }

    // 唤醒所有任务后，尝试进行调试
    os_protect_t protect = os_enter_protect();
    int cnt = os_event_clear(&mutex->event, OS_NULL, OS_ERR_REMOVE);
    os_leave_protect(protect);

    // 有任务唤醒时，进行调度，以便运行更高优先级的任务
    if (cnt > 0) {
        os_dbg("%d task wakeup, scheduler\n", cnt);
        os_sched_run();
    } 
    return OS_ERR_OK;
}

/**
 * 分配一个信号量
 * @param init_cnt 信号量初始值
 * @param max_cnt 信号量最大计数值, 如果为0时表示没有限制
 * @return 创建的信号量
 */
os_mutex_t * os_mutex_create (void) {
    // 分配内存
    os_mutex_t * mutex = os_mem_alloc(sizeof(os_mutex_t));
    if (mutex == OS_NULL) {
        os_dbg("error: alloc mutex failed");
        return OS_NULL;
    }

    // 初始化结构
    os_err_t err = mutex_init(mutex, OS_EVENT_FLAG_ALLOC);
    if (err < 0) {
        os_dbg("error: init mutex failed.");
        os_mem_free(mutex);
        return OS_NULL;
    }

    return mutex;
}

/**
 * 释放一个信号量
 * @param 需要释放的信号量
 */
os_err_t os_mutex_free (os_mutex_t * mutex) {
    os_param_failed(mutex == OS_NULL, OS_ERR_PARAM);

    os_mutex_uninit(mutex);
    os_mem_free(mutex);
    return OS_ERR_OK;
}

/**
 * 互斥信号量上锁
 * @param mutex 操作的互斥信号量
 * @param ms 等待的时间
 * @return 上锁结果，< 0表示超时或其它错误
*/
os_err_t os_mutex_lock (os_mutex_t * mutex, int ms) {
    os_param_failed(mutex == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();

    // 没有其它任务锁定时，自己给上锁一下
    if (mutex->owner == OS_NULL) {
        mutex->lock_cnt = 1;
        mutex->owner = os_task_self();
        mutex->raw_prio = mutex->owner->prio;
        os_leave_protect(protect);
        return OS_ERR_OK;
    } 
    
    // 自己反复锁定
    if (mutex->owner == os_task_self()) {
        mutex->lock_cnt++;
        os_dbg("mutex locked cnt: %d > 1\n", mutex->lock_cnt);
        os_leave_protect(protect);

        return OS_ERR_OK;
    } 

    // 是否要等待
    if (ms < 0) {
        os_dbg("mutex locked and no wait\n");
        os_leave_protect(protect);
        return OS_ERR_LOCKED;
    }
    
    // 有其它任务在锁定，等待该任务。采用优先级继承方式处理优先级翻转的问题
    os_task_t * self = os_task_self();
    os_task_t * owner = mutex->owner;
    if (self->prio < owner->prio) {
        os_dbg("mutex inherit prio happen: task %s, prio from %d to %d\n", 
            owner->name, owner->prio, self->prio);
        os_task_t * owner = mutex->owner;

        // 如果当前任务的优先级比拥有者优先级更高，则使用优先级继承, 提升其优先级
        // 可能会碰到多次提升的情况
        if ((owner->flags & OS_TASK_FLAG_DELAY) || owner->wait) {
            owner->prio = self->prio;
        } else {
            os_sched_set_unready(owner);
            owner->prio = self->prio;
            os_sched_set_ready(owner);
        }
    }
    os_leave_protect(protect);

    // 当前任务进入等待队列中等, TODO: 按优先级等待
    os_err_t err = os_event_wait(&mutex->event, OS_NULL, ms);
    return err;
}

/**
 * 尝试解锁信号量
 * @param 待解锁的信号量
 * @return 解锁结果，OK解锁成功，其它解锁失败
 */
os_err_t os_mutex_unlock (os_mutex_t * mutex) {
    os_param_failed(mutex == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();

    // 已经为非锁定状态
    if (mutex->owner == OS_NULL) {
        os_leave_protect(protect);
        return OS_ERR_UNLOCKED;
    }

    // 不是拥有者释放，认为是非法
    os_task_t * self = os_task_self();
    if (mutex->owner != self) {
        os_dbg("mutex.owner != self\n");
        os_leave_protect(protect);
        return OS_ERR_OWNER;
    }

    // 减1后计数仍不为0, 直接退出，不需要唤醒等待的任务
    if (--mutex->lock_cnt > 0) {
        os_dbg("mutex.lock_cnt %d > 1\n", mutex->lock_cnt);
        os_leave_protect(protect);
        return OS_ERR_OK;
    }

    // 是否有发生优先级继承，如果是则需要恢复优先级
    if (mutex->raw_prio != self->prio) {
        os_dbg("mutex inherit prio happen: task %s, prio from %d to %d\n",
            self->name, self->prio,  mutex->raw_prio);

        // 有发生优先级继承，恢复拥有者的优先级
        if ((self->flags & OS_TASK_FLAG_DELAY) || self->wait) {
            self->prio = mutex->raw_prio;
        } else {
            os_sched_set_unready(self);
            self->prio = mutex->raw_prio;
            os_sched_set_ready(self);
        }
    }

    // 唤醒等待的优先级最高的任务拥有该锁
    os_task_t * task = os_event_wakeup(&mutex->event);
    if (task) {
        // 任务已经不再等待，清除
        os_wait_t * wait = task->wait;
        wait->err = OS_ERR_OK;
        task->wait = OS_NULL;

        // 设置信号量        
        mutex->owner = task;
        mutex->raw_prio = task->prio;
        mutex->lock_cnt = 1;
        os_leave_protect(protect);

        // 如果这个任务的优先级更高，就执行调度，切换过去
        os_sched_run();
    } else {
        mutex->owner = OS_NULL;
        os_leave_protect(protect);
    }
    return OS_ERR_OK;
}

/**
 * 返回互斥信号量当前上锁的计数值
 * @param 互斥信号量
 * @return 当前上锁的计数值
*/
int os_mutex_lock_cnt (os_mutex_t * mutex) {
    os_param_failed(mutex == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = mutex->lock_cnt;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 返回互斥信号量当前拥有者
 * @param 互斥信号量
 * @return 当前互斥信号量的拥有者
*/
os_task_t * os_mutex_owner (os_mutex_t * mutex) {
    os_param_failed(mutex == OS_NULL, OS_NULL);

    os_protect_t protect = os_enter_protect();
    os_task_t * owner = mutex->owner;
    os_leave_protect(protect);
    return owner;
}

/**
 * 获取互斥信号量的任务数量
 * @param sem 信号量
 * @return 互斥锁的最大计数
 */
int os_mutex_tasks (os_mutex_t * mutex) {
    os_param_failed(mutex == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&mutex->event);
    os_leave_protect(protect);
    return cnt;
}

#endif


