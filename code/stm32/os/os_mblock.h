#ifndef OS_MBLOCK_H
#define OS_MBLOCK_H

#include "os_cfg.h"
#include "os_event.h"
#include "os_core.h"

/**
 * 内存块链表结点
 */
typedef struct _mem_link_t {
    struct _mem_link_t * next;
}mem_link_t;

/**
 * 定长存储块结构
 */
typedef struct _os_mblock_t {
    os_event_t event;           // 事件控制块
    void * mem;                 // 存储起始地址
    mem_link_t * list;          // 块链表
    int blk_size;               // 块大小
    int blk_cnt;                // 块数量
    int blk_max;                // 最大的块数量
}os_mblock_t;

os_err_t os_mblock_init (os_mblock_t * mblock, void * mem, int blk_size, int blk_cnt);
os_err_t os_mblock_uninit (os_mblock_t * mblock);
os_mblock_t * os_mblock_create (int blk_size, int blk_cnt);
os_err_t os_mblock_free (os_mblock_t * mblock);

void * os_mblock_wait(os_mblock_t * mblock, int ms, os_err_t * p_err);
os_err_t os_mblock_release (os_mblock_t * mblock, void * mem);

int os_mblock_blk_cnt (os_mblock_t * mblock);
int os_mblock_blk_size (os_mblock_t * mblock);
int os_mblock_tasks(os_mblock_t * mblock);

#endif /* TMEMBLOCK_H */
