#include "os_eflags.h"
#include "os_mem.h"
#include "os_list.h"

typedef struct _eflags_wait_t {
    int type;
    os_flags_t flags;
    os_flags_t * result;
}eflags_wait_t;

/**
 * 初始化事件标志
 * @param eflags 事件标志
 * @param init_flags 初始的标志位
*/
os_err_t eflags_init (os_eflags_t * eflags, os_flags_t init_flags, int flags) {
    // 初始化等待结构
    os_err_t err = os_event_init(&eflags->event, OS_EVENT_TYPE_EFLAGS, flags);
    if (err < 0) {
        os_dbg("create eflags event failed: %s.", os_err_str(err));
        return err;
    }

    eflags->flags = init_flags;
    return OS_ERR_OK;
}

/**
 * 初始化事件标志
 * @param eflags 事件标志
 * @param init_flags 初始的标志位
*/
os_err_t os_eflags_init (os_eflags_t * eflags, os_flags_t init_flags) {
    os_param_failed(eflags == OS_NULL, OS_ERR_PARAM);

    return eflags_init(eflags, init_flags, 0);
}

/**
 * 取消事件标志的初始化
 * @param eflags 事件标志
*/
os_err_t os_eflags_uninit (os_eflags_t * eflags) {       
    os_param_failed(eflags == OS_NULL, OS_ERR_PARAM);

    // 唤醒所有任务后，尝试进行调试
    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&eflags->event);
    os_event_uninit(&eflags->event);
    os_leave_protect(protect);

    // 有任务唤醒时，进行调度，以便运行更高优先级的任务
    if (cnt > 0) {
        os_sched_run();
    } 

    return OS_ERR_OK;
}

/**
 * 分配一个事件标志
 * @param init_cnt 事件标志初始值
 * @param max_cnt 事件标志最大计数值, 如果为0时表示没有限制
 * @return 创建的事件标志
 */
os_eflags_t * os_eflags_create (os_flags_t init_flags) {
    // 分配内存
    os_eflags_t * eflags = os_mem_alloc(sizeof(os_eflags_t));
    if (eflags == OS_NULL) {
        os_dbg("error: alloc eflags failed");
        return OS_NULL;
    }

    // 初始化事件标志结构
    os_err_t err = eflags_init(eflags, init_flags, OS_EVENT_FLAG_ALLOC);
    if (err < 0) {
        os_dbg("error: init eflags failed.");
        os_mem_free(eflags);
        return OS_NULL;
    }

    return eflags;
}

/**
 * 释放一个事件标志
 * @param 需要释放的事件标志
 */
os_err_t os_eflags_free (os_eflags_t * eflags) {
    os_param_failed(eflags == OS_NULL, OS_ERR_PARAM);

    os_eflags_uninit(eflags);
    if (eflags->event.flags & OS_EVENT_FLAG_ALLOC) {
        os_mem_free(eflags);
    }

    return OS_ERR_OK;
}

/**
 * 检查相关的标志是否满足要求
*/
static os_flags_t eflags_check (os_eflags_t * eflags, int type, os_flags_t cflags, os_err_t * p_err) {
	int clear = type & OS_EOPT_EXIT_CLEAR;
    int check_all = type & OS_EOPT_ALL;

    os_flags_t flags = 0;
    os_err_t err = OS_ERR_NONE;
    if (type & OS_EOPT_SET) {
        // 置1检查
        flags = eflags->flags & cflags;      // 取出哪些位置1了
        if ((check_all && (flags == cflags)) || (!check_all && flags)) {
            // 检查所有位，且所有的要检查位的位都置1。或者检查任意位，且任意位为1
            if (clear) {
                eflags->flags &= ~flags;               // 清0取得的这些位
            }
            err = OS_ERR_OK;
        } 
    } else {
        // 清0检查
        flags = ~eflags->flags & cflags;     // 取出哪些位已经清零了
        if ((check_all && (flags == cflags)) || (!check_all && flags)) {
            if (clear) {
                eflags->flags |= flags;     // 将这些位置1
            }
            err = OS_ERR_OK;
        }
    }

    if (p_err) {
        *p_err = err;
    }
    return flags;
}

/**
 *  等待事件标志组中特定的标志
 * @param eflags 等待的事件标志组
 * @param ms 等待的时间，毫秒为单位
 * @param type 等待的方式
 * @param wait 等待的标志位
 * @param result 当满足条件时的相应结果
*/
os_flags_t os_eflags_wait (os_eflags_t * eflags, int ms, int type, os_flags_t flags,  os_err_t * p_err) {
    os_param_failed_exec(eflags == OS_NULL, 0, if (p_err) *p_err = OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();

    os_err_t err;
    os_flags_t result_flags = eflags_check(eflags, type, flags, &err);
    if (err == OS_ERR_OK) {
        //  等待条件已经满足，退出
        os_leave_protect(protect);
    } else {
        os_leave_protect(protect);
        
        // 请求等待结果
        eflags_wait_t wait;
        wait.result = &result_flags;
        wait.type = type;
        wait.flags = flags;
        err = os_event_wait(&eflags->event, &wait, ms);
    }

    if (p_err) {
        *p_err = err;
    }
    return result_flags;
}

/**
 * 通知事件标志组中的任务有新的标志发生
 * @param eflags 事件标志
 * @param type 释放的事件类型
 * @param flags 释放的标志位
 */
os_err_t os_eflags_release (os_eflags_t * eflags, int type, os_flags_t flags) {
    uint32_t status = os_enter_protect();

    // 先清除或置位其听 标志位
    if (type & OS_EOPT_SET) {
        eflags->flags |= flags;
    } else {
        eflags->flags &= ~flags;
    }

    // 遍历所有的等待任务, 获取满足条件的任务，加入到待移除列表中
    int need_sched = 0;
    os_wait_t * curr = eflags->event.first;
    while (curr) {
        os_wait_t * next = curr->next;
        os_task_t * task = curr->task;

        os_err_t err;
        eflags_wait_t * wait = (eflags_wait_t *)curr->reason;
        *wait->result = eflags_check(eflags, wait->type, wait->flags, &err);
        if (err == OS_ERR_OK) {
            os_event_wakeup_task(&eflags->event, task, OS_NULL, OS_ERR_OK);
            need_sched = 1;
        } 

        curr = next;
    }

    // 如果有任务就绪，则执行一次调度
    if (need_sched) {
        os_sched_run();
    }

    os_leave_protect(status);
    return OS_ERR_OK;
}

/**
 * 获取事件标志当前计数
 * @param eflags 操作的事件标志
 * @return 当前的事件标志
 */
os_flags_t os_eflags_flags (os_eflags_t * eflags, os_err_t * err) {
    os_param_failed_exec(eflags == OS_NULL, OS_ERR_PARAM, if (err) *err = OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    os_flags_t flags = eflags->flags;
    os_leave_protect(protect);

    *err = OS_ERR_OK;
    return flags;
}

/**
 * 获取等待事件标志的任务数量
 * @param eflags 事件标志
 * @return 事件标志的最大计数
 */
int os_eflags_tasks (os_eflags_t * eflags) {
    os_param_failed(eflags == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&eflags->event);
    os_leave_protect(protect);
    return cnt;
}


