#ifndef OS_KLIB_H
#define OS_KLIB_H

#include "os_plat.h"
#include "os_err.h"
#include "os_dbg.h"

#define OS_NULL         (void *)0               // 空指针

/**
 * 任务队列
*/
struct _os_task_t;
typedef struct _os_list_t {   
    struct _os_task_t * first;          // 首任务   
    struct _os_task_t * last;           // 末任务
	int cnt;                            // 队列中任务数量
}os_list_t;

void os_list_init (os_list_t * list);
struct _os_task_t * os_list_first (os_list_t * list);
struct _os_task_t * os_list_last (os_list_t * list);
struct _os_task_t * os_list_next (os_list_t * list, struct _os_task_t * task);

void os_list_clear (os_list_t * list);
void os_list_add_head (os_list_t * list, struct _os_task_t * task);
void os_list_append (os_list_t * list, struct _os_task_t * task);
struct _os_task_t * os_list_remove_head (os_list_t * list);
void os_list_remove (os_list_t * list, struct _os_task_t * task);
void os_list_insert_after (os_list_t * list, struct _os_task_t * pre, struct _os_task_t * next);

#if OS_DEBUG_ENABLE && OS_ENABLE_DEBUG_LIST
void os_list_show (os_list_t * list,  const char * title);
#else
#define os_list_show(list, title)
#endif 

const char * os_err_str (os_err_t err);

#endif /* TLIB_H */
