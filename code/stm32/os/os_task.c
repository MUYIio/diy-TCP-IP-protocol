#include "os_task.h"
#include "os_list.h"
#include "os_plat.h"
#include "os_core.h"
#include "os_mem.h"

/**
 * 初始化一个任务
 * @param task 任务结构 
 * @param entry 任务入口函数
 * @param param 运行参数
 * @param prio 任务运行优先级
 * @param stack 栈的起始地址，内存的起始地址
 * @param stack_size 栈的大小
*/
os_err_t os_task_init (os_task_t * task, const char * name, task_entry_t entry, void *param, 
                    int prio, void * stack, int stack_size) {
    os_param_failed(task == OS_NULL, OS_ERR_PARAM);
    os_param_failed(entry == OS_NULL, OS_ERR_PARAM);
    os_param_failed((prio < 0) && (prio >= OS_PRIO_CNT), OS_ERR_PARAM);
    os_param_failed(stack == OS_NULL, OS_ERR_PARAM);
    os_param_failed(stack_size <= sizeof(os_task_ctx_t), OS_ERR_PARAM);

    task->name = name;

    // 初始化栈空间
    task->start_stack = stack;
    task->stack_size = stack_size;
    task->slice = OS_TASK_SLICE;
    memset(stack, 0, stack_size);

    task->prio = prio; 

    // 初始化任务的运行环境
    os_task_ctx_init(task, entry, param);
    return OS_ERR_OK;
}

os_task_t * os_task_create (const char * name, task_entry_t entry, void *param, 
                    int prio, int stack_size) {
    os_param_failed(entry == OS_NULL, OS_NULL);
    os_param_failed((prio < 0) && (prio >= OS_PRIO_CNT), OS_NULL);
    os_param_failed(stack_size <= sizeof(os_task_ctx_t), OS_NULL);

    // 分配内存
    os_task_t * task = (os_task_t *)os_mem_alloc(sizeof(os_task_t));
    if (task == OS_NULL) {
        os_dbg("error: alloc task failed");
        return OS_NULL;
    }

    stack_size = stack_size & ~(sizeof(cpu_stack_t) - 1);
    char * stack = (char *)os_mem_alloc(stack_size);
    if (stack == OS_NULL) {
        os_dbg("error: alloc stack failed");
        goto create_failed;
    }

    // 初始化task
    os_err_t err = os_task_init(task, name, entry, param, prio, stack, stack_size);
    if (err < 0) {
        os_dbg("error: init task failed.");
        goto create_failed;
    }

    return task;
create_failed:
    os_mem_free(task);
    if (stack) {
        os_mem_free(stack);
    }
    return OS_NULL;
}

/**
 * 启动创建好的任务开始运行
 */
void os_task_start (os_task_t * task) {
    // 插入就绪队列
    os_protect_t protect = os_enter_protect();

    task_insert_os(task);
    os_sched_set_ready(task);

    // 进行一次调度
    if (os_task_self() != OS_NULL) {
        os_sched_run();
    }
    os_leave_protect(protect); 
}

/**
 * 结束当前任务运行
 * 该函数应当永远不会返回，且任务应当已经自行完成了一些清理工作
 * 任务块的释放，交由其它任务完成：TODO:
*/
void os_task_exit (void) {
    os_protect_t protect = os_enter_protect();

    // 从就绪队列中移除
    os_task_t * self = os_task_self();
    os_sched_set_unready(self);
    os_task_insert_delete(self);

    // 进行一次调度
    os_sched_run();
    os_leave_protect(protect); 
    while (1);
}

/**
 * 让当前任务延时指定毫秒
 * @param ms 延时的毫秒数
*/
void os_task_sleep (int ms) {
    os_protect_t protect = os_enter_protect();
    if (ms == 0) {
        os_leave_protect(protect);
        return;
    }

    // 从就绪队列移到延时队列，进行调度切换到其它任务运行
    os_task_t * self = os_task_self();
    os_sched_set_unready(self);
    os_task_set_delay(self, ms);

    // 切换到其它任务运行
    os_sched_run();
    os_leave_protect(protect); 
}

