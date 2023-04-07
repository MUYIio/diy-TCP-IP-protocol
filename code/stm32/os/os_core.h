#ifndef OS_CORE_H
#define OS_CORE_H

#include "os_err.h"
#include "os_cfg.h"
#include "os_plat.h"
#include "os_list.h"

typedef struct _os_task_t os_task_t;

/**
 * 位图类型
 * 用于内核任务的优先级组织，支持32个优先级，基本是够用了
*/
typedef struct _os_bitmap_t {
    uint8_t group;              // 优先级组位图
	uint8_t map[8];             // 各分组位图
}os_bitmap_t;

/**
 * 内核核心数据结构
 * 不放在头文件，不然会有很多文件包含方面的问题
 */
typedef struct _os_core_t {
    os_task_t * curr_task;                  // 当前正在运行的任务
    os_task_t * next_task;                  // 下一要运行的任务

    os_bitmap_t task_map;                   // 优先级位图
    os_list_t task_list[OS_PRIO_CNT];      // 就绪队列表
    os_list_t delay_list;                   // 等待队列
    os_list_t delete_list;                  // 待删除的任务队列
    os_list_t all_list;                     // 所有任务

    // 用于空闲任务的任务结构和堆栈空间
    os_task_t * idle_task;
    
    int locked;                          // 调度锁锁定的计数值

    // 时间相关
    int ticks;                          // 时钟节拍计数值  

    int task_cnt; 
}os_core_t;

void os_sched_disable (void);
void os_sched_run (void);
void os_sched_enable (void);
void os_sched_set_ready (os_task_t * task);
void os_sched_set_unready (os_task_t * task);
void os_task_set_delay (os_task_t * task, int ms);
void os_task_set_wakeup (os_task_t * task);
void os_time_tick (void);
os_task_t * os_task_self (void);
int os_tick_count(void);

void os_task_set_curr(os_task_t * task_to);
void os_task_insert_delete(os_task_t * task);

os_task_ctx_t *os_switch_ctx(os_task_ctx_t *ctx);
os_err_t os_init (void);
void os_start (void);

#endif // OS_CORE_H
