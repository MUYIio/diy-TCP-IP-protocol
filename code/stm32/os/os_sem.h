#ifndef OS_SEM_H
#define OS_SEM_H

#include "os_cfg.h"

#if OS_SEM_ENABLE == 1

#include "os_err.h"
#include "os_event.h"

/**
 * 信号量
 */
typedef struct _os_sem_t  {
	os_event_t event;			// 事件结构
	int curr_cnt;				// 当前计数值
	int max_cnt;					// 最大计数值
}os_sem_t;

os_err_t os_sem_init (os_sem_t * sem, int init_cnt, int max_cnt);
os_err_t os_sem_uninit(os_sem_t * sem);
os_sem_t * os_sem_create (int init_cnt, int max_cnt);
os_err_t os_sem_free (os_sem_t * sem);
os_err_t os_sem_wait (os_sem_t * sem, int ms);
os_err_t os_sem_release (os_sem_t * sem);
int os_sem_cnt (os_sem_t * sem);
int os_sem_max (os_sem_t * sem);
int os_sem_tasks (os_sem_t * sem);

#endif

#endif /* TSEM_H */
