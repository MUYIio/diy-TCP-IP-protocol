/**
 * 计数信号量
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_SEM_H
#define OS_SEM_H

#include "tools/list.h"

/**
 * 进程同步用的计数信号量
 */
typedef struct _sem_t {
    int count;				// 信号量计数
    list_t wait_list;		// 等待的进程列表
}sem_t;

void sem_init (sem_t * sem, int init_count);
void sem_wait (sem_t * sem);
int sem_wait_tmo (sem_t * sem, int ms);
void sem_notify (sem_t * sem);
int sem_count (sem_t * sem);

#endif //OS_SEM_H
