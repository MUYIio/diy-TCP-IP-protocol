#ifndef OS_EVENT_H
#define OS_EVENT_H

#include "os_cfg.h"
#include "os_list.h"
#include "os_task.h"
#include "os_core.h"

/**
 * event类型，用于让任务进行各种等待的事件处理
 */
typedef enum  _os_event_type_t {   
    OS_EVENT_TYPE_INVALID, 		    // 未知类型
    OS_EVENT_TYPE_SEM, 				// 信号量类型
    OS_EVENT_TYPE_QUEUE, 		    // 消息队列类型
	OS_EVENT_TYPE_MBLOCK,		    // 存储块类型
	OS_EVENT_TYPE_EFLAGS,		    // 事件标志组
	OS_EVENT_TYPE_MUTEX,		    // 互斥信号量类型

    OS_EVENT_TYPE_MAX               // 最大无效值
}os_event_type_t;

#define OS_EVENT_WAIT_PRIO          (1 << 0)        // 按优先级进行排序
#define OS_EVENT_FLAG_ALLOC         (1 << 1)        // 采用的动态分配

struct _os_wait_t;

/**
 * event结构，用于任务等待与恢复相关的控制
*/
typedef struct _os_event_t {
    struct {
        int type : 4;                       // 类型
        int flags : 4;                      // 一些标志位
    };

    // 用双向链表，方便快速进行开头和尾部的操作
    // 中间的操作情况较少，为节省内存，采用从头部遍历的方式
    struct _os_wait_t * first;	            // 前一指针
    struct _os_wait_t * last;	            // 后一指针
}os_event_t;

/**
 * 任务等待的对像结构
 */
typedef struct _os_wait_t {
    struct _os_wait_t * next;       // 前后同样等待的任务
    os_event_t * event;             // 等待的
    os_task_t * task;               // 当前等待的任务
    void * reason;                  // 等待的原因
    os_err_t err;                   // 等待结果
}os_wait_t;

void os_wait_init (os_wait_t * wait, void * reason, int ms);

os_err_t os_event_init (os_event_t * event, os_event_type_t type, int flags);
os_err_t os_event_uninit (os_event_t * event);
os_err_t os_event_wait (os_event_t * event, void * reason, int ms);
os_task_t * os_event_wakeup (os_event_t * event);
void os_event_wakeup_task (os_event_t * event, os_task_t * task, void * reason, os_err_t err);
int os_event_clear (os_event_t * event, void * reason, os_err_t err);
int os_event_wait_cnt (os_event_t * event);

#endif /* TEVENT_H */
