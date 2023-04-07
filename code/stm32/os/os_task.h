#ifndef OS_TASK_H
#define OS_TASK_H

#include "os_plat.h"
#include "os_err.h"

#define	TINYOS_TASK_STATE_RDY					0
#define TINYOS_TASK_STATE_DESTROYED             (1 << 0)
#define	TINYOS_TASK_STATE_DELAYED				(1 << 1)
#define TINYOS_TASK_STATE_SUSPEND               (1 << 2)

#define TINYOS_TASK_WAIT_MASK                   (0xFF << 16)

#define OS_TASK_FLAG_DELAY                  (1 << 0)

struct _os_wait_t;

/**
 * 任务运行的入口函数
 */
typedef void (*task_entry_t)(void *);

/**
 * 任务结构
 */
typedef struct _os_task_t {
    const char * name;                  // 任务名称
    os_task_ctx_t * ctx;                // 任务的当前上下文

    cpu_stack_t * start_stack;          // 栈的起始地址
    int stack_size;                     // 栈的大小
    
    int delay_tick;                     // 延时计数器
    int prio;                           // 优先级
    int flags;                          // 一些标志位
    int slice;                          // 时间片
    struct _os_task_t * pre;            // 前一任务
    struct _os_task_t * next;           // 后一任务

    struct _os_wait_t * wait;           // 等待的事件
}os_task_t;


os_task_t * os_task_create (const char * name, task_entry_t entry, void *param, 
                    int prio, int stack_size);
os_err_t os_task_init (os_task_t * task, const char * name, task_entry_t entry, void *param, 
                int prio, void * stack, int stack_size);
void os_task_start (os_task_t * task);

void os_task_yield (void);

os_task_t * os_task_alloc(task_entry_t * entry, void *param, int prio, int stack_size);
os_task_t * os_task_self (void);
void os_task_exit (void);
void os_task_sleep (int ms);

void os_idle_create (void);


#endif /* OS_TASK_H */ 
