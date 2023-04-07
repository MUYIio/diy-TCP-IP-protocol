
/**
 * 操作系统接口相关定义及消息队列的实现
 */
#ifndef NET_SYS_H
#define NET_SYS_H

#include "net_plat.h"

void sys_time_curr (net_time_t * time);
int sys_time_goes (net_time_t * pre);

// 计数信号量相关：由具体平台实现
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

#endif // NET_SYS_H
