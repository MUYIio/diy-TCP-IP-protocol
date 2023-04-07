
#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#include "os_cfg.h"
#include "os_event.h"
#include "os_core.h"

/**
 * 互斥信号量
 */
typedef struct  _os_mutex_t {
    os_event_t event;           // 事件控制块
    int lock_cnt;               // 锁定次数
    os_task_t * owner;          // 拥有者
    int raw_prio;             // 拥有者原始优先级
}os_mutex_t;

os_err_t os_mutex_init (os_mutex_t * mutex);
os_err_t os_mutex_uninit (os_mutex_t * mutex);       
os_mutex_t * os_mutex_create (void);
os_err_t os_mutex_free (os_mutex_t * mutex);
os_err_t os_mutex_lock (os_mutex_t * mutex, int ms);
os_err_t os_mutex_unlock (os_mutex_t * mutex);
int os_mutex_lock_cnt (os_mutex_t * mutex);
int os_mutex_tasks (os_mutex_t * mutex);
os_task_t * os_mutex_owner (os_mutex_t * mutex);

#endif /* TMUTEX_H */
