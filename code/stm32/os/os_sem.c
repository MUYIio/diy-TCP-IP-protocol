#include "os_core.h"
#include "os_sem.h"
#include "os_mem.h"
#include "os_list.h"

/**
 * 初始化一个信号量，该信号量已经提前分配好
 * @param sem 需要初始化的信号量
 * @param init_cnt 信号量初始值
 * @param max_cnt 信号量最大计数值, 小于0表示没限制
 * @param flags 初始化相关参数
 * @return 初始化结果， < 0表示出错
*/
static os_err_t sem_init (os_sem_t * sem, int init_cnt, int max_cnt, int flags) {
    // 初始化等待结构
    os_err_t err = os_event_init(&sem->event, OS_EVENT_TYPE_SEM, flags);
    if (err < 0) {
        os_dbg("create sem event failed: %s.", os_err_str(err));
        return err;
    }

    // 设置各计数值
    if ((max_cnt > 0) && (init_cnt > max_cnt)) {
        os_dbg("warning: init_cnt(%d) > max_cnt(%d)", init_cnt, max_cnt);
        init_cnt = max_cnt;
    }

    sem->curr_cnt = init_cnt;
    sem->max_cnt = max_cnt;
    return OS_ERR_OK;
}

/**
 * 初始化一个信号量，该信号量已经提前分配好
 * @param sem 需要初始化的信号量
 * @param init_cnt 信号量初始值
 * @param max_cnt 信号量最大计数值, 小于0表示没限制
 * @return 初始化结果， < 0表示出错
*/
os_err_t os_sem_init (os_sem_t * sem, int init_cnt, int max_cnt) {
    // 必要的参数检查
    os_param_failed(sem == OS_NULL, OS_ERR_PARAM);
    os_param_failed(init_cnt < 0, OS_ERR_PARAM);
    os_param_failed(max_cnt < 0, OS_ERR_PARAM);
    return sem_init(sem, init_cnt, max_cnt, 0);
}

/**
 * 取消信号量的初始化
 * @param sem 信号量
*/
os_err_t os_sem_uninit (os_sem_t * sem) {       
    os_param_failed(sem == OS_NULL, OS_ERR_PARAM);

    // 唤醒所有任务后，尝试进行调试
    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&sem->event);
    os_event_uninit(&sem->event);
    os_leave_protect(protect);

    // 有任务唤醒时，进行调度，以便运行更高优先级的任务
    if (cnt > 0) {
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
os_sem_t * os_sem_create (int init_cnt, int max_cnt) {
    os_param_failed(init_cnt < 0, OS_NULL);

    // 分配内存
    os_sem_t * sem = os_mem_alloc(sizeof(os_sem_t));
    if (sem == OS_NULL) {
        os_dbg("error: alloc sem failed");
        return OS_NULL;
    }

    // 初始化信号量结构
    os_err_t err = sem_init(sem, init_cnt, max_cnt, OS_EVENT_FLAG_ALLOC);
    if (err < 0) {
        os_dbg("error: init sem failed.");
        os_mem_free(sem);
        return OS_NULL;
    }

    return sem;
}

/**
 * 释放一个信号量
 * @param 需要释放的信号量
 */
os_err_t os_sem_free (os_sem_t * sem) {
    os_param_failed(sem == OS_NULL, OS_ERR_PARAM);

    os_sem_uninit(sem);
    if (sem->event.flags & OS_EVENT_FLAG_ALLOC) {
        os_mem_free(sem);
    }

    return OS_ERR_OK;
}

/**
 * 获取信号量当前计数
 * @param sem 操作的信号量
 * @return 信号量的计数
 */
int os_sem_cnt (os_sem_t * sem) {
    os_param_failed(sem == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = sem->curr_cnt;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取信号量最大计数
 * @param sem 信号量
 * @return 信号量的最大计数
 */
int os_sem_max (os_sem_t * sem) {
    os_param_failed(sem == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = sem->max_cnt;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取等待信号量的任务数量
 * @param sem 信号量
 * @return 信号量的最大计数
 */
int os_sem_tasks (os_sem_t * sem) {
    os_param_failed(sem == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&sem->event);
    os_leave_protect(protect);
    return cnt;
}

/**
 * 等待信号量可用，获取一个计数
 * @param sem 信号量
 * @param ms 等待的超时值，为0表示无限等待
 * @return 等待结果，没问题时返回OK，超时时返回TMO，其它返回相应错误值
*/
os_err_t os_sem_wait (os_sem_t * sem, int ms) {
    os_param_failed(sem == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    if (sem->curr_cnt > 0) {
        sem->curr_cnt--;
        os_leave_protect(protect);
        return OS_ERR_OK;
    } else if (ms < 0) {
        os_dbg("sem count == 0 and no wait\n");
        os_leave_protect(protect);
        return OS_ERR_NONE;
    } else {
        os_leave_protect(protect);
        
        os_err_t err = os_event_wait(&sem->event, OS_NULL, ms);
        return err;
    }
}

/**
 * 通知信号量可用，唤醒等待队列中的一个任务，或者将计数+1
 * @param sem 操作的信号量
 * @return 无问题，返回OK
 */
os_err_t os_sem_release (os_sem_t * sem) {
    os_param_failed(sem == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    os_task_t * task = os_event_wakeup(&sem->event);
    if (!task) {
        // 没有任务在等时，增加计数。如果超出最大值，则警告并限制不超出
        sem->curr_cnt++;
        if ((sem->max_cnt > 0) && (sem->curr_cnt > sem->max_cnt)) {
            os_dbg("warning: sem->curr_cnt(%d) > sem->max_cnt(%d)", sem->curr_cnt, sem->max_cnt);
            sem->curr_cnt = sem->max_cnt;
        }
        os_leave_protect(protect);
    } else {
        // 任务已经不再等待，清除
        os_wait_t * wait = task->wait;
        wait->err = OS_ERR_OK;
        task->wait = OS_NULL;

        os_leave_protect(protect);

        // 有任务等，通知调度器，看看是否要进行任务切换
        os_sched_run();
    }
	
	return OS_ERR_OK;
}

