/**
 * @file mblock.h
 * @author lishutong (527676163@qq.com)
 * @brief 存储块管理器
 * 用于将固定大小的存储块组织成块链表，并允许从中申请和释放块
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef MBLOCK_H
#define MBLOCK_H

#include "nlist.h"
#include "nlocker.h"

/**
 * @brief 存储管块管理器
 */
typedef struct _mblock_t{
    void* start;                        // 所有存储的起始地址
    nlist_t free_list;                   // 空闲的消息队列
    nlocker_t locker;                   // 访问的锁
    sys_sem_t alloc_sem;                // 分配同步用信号量
}mblock_t;

net_err_t mblock_init (mblock_t* mblock, void * mem, int blk_size, int cnt, nlocker_type_t share_type);
void * mblock_alloc(mblock_t * block, int ms);
int mblock_free_cnt(mblock_t* list);
void mblock_free(mblock_t * list, void * block);
void mblock_destroy(mblock_t* block);

#endif // MBLOCK_MGR_H
