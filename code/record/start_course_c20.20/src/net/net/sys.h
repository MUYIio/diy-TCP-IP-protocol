#ifndef SYS_H
#define SYS_H

#include "net_plat.h"

void sys_time_curr (net_time_t * time);
int sys_time_goes (net_time_t * pre);

sys_sem_t sys_sem_create(int init_count);
void sys_sem_free(sys_sem_t sem);
int sys_sem_wait(sys_sem_t sem, uint32_t ms);
void sys_sem_notify(sys_sem_t sem);

// 互斥信号量：由具体平台实现
sys_mutex_t sys_mutex_create(void);
void sys_mutex_free(sys_mutex_t mutex);
void sys_mutex_lock(sys_mutex_t mutex);
void sys_mutex_unlock(sys_mutex_t mutex);
int sys_mutex_is_valid(sys_mutex_t mutex);

// 线程相关：由具体平台实现
typedef void (*sys_thread_func_t)(void * arg);
sys_thread_t sys_thread_create(sys_thread_func_t entry, void* arg);
void sys_thread_exit (int error);
void sys_sleep(int ms);
sys_thread_t sys_thread_self (void);

#endif // 