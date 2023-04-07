﻿
/**
 * 网络数据包分配管理实现
 *
 * 从网络上接收到的数据帧是变成的，用户要发送的数据也是变成的。所以需要
 * 一种灵活的方式来适应这种变长，以避免需要使用一个超大的内存块来保存数据，减少空间的浪费
 * 这里提供的是一种基于链式的存储方式，将原本需要一大块内存存储的数据，分块存储在多个数据块中
 * 数据量少时，需要的块就少；数据量大时，需要的块就多；提高了存储利用率。
 */
#include <string.h>
#include "net_err.h"
#include "pktbuf.h"
#include "mblock.h"
#include "dbg.h"

static nlocker_t locker;                    // 分配与回收的锁
static mblock_t block_list;                 // 空闲包列表
static pktblk_t block_buffer[PKTBUF_BLK_CNT];
static mblock_t pktbuf_list;                // 空闲包列表
static pktbuf_t pktbuf_buffer[PKTBUF_BUF_CNT];

/**
 * 获取blk的剩余空间大小
 */
static inline int curr_blk_tail_free(pktblk_t* blk) {
    return PKTBUF_BLK_SIZE - (int)(blk->data - blk->payload) - blk->size;
}

/**
 * 打印缓冲表链、同时检查表链的是否正确配置
 *
 * 在打印的过程中，同时对整个缓存链表进行检查，看看是否存在错误
 * 主要通过检查空间和size的设置是否正确来判断缓存是否正确设置
 *
 * @param buf 待查询的Buf
 */
#if DBG_DISP_ENABLED(DBG_BUF)
static void display_check_buf(pktbuf_t* buf) {
    if (!buf) {
        dbg_error(DBG_BUF, "invalid buf. buf == 0");
        return;
    }

    plat_printf("check buf %p: size %d\n", buf, buf->total_size);
    pktblk_t* curr;
    int total_size = 0, index = 0;
    for (curr = pktbuf_first_blk(buf); curr; curr = pktbuf_blk_next(curr)) {
        plat_printf("%d: ", index++);

        if ((curr->data < curr->payload) || (curr->data >= curr->payload + PKTBUF_BLK_SIZE)) {
            dbg_error(DBG_BUF, "bad block data. ");
        }

        int pre_size = (int)(curr->data - curr->payload);
        plat_printf("Pre:%d b, ", pre_size);

        // 中间存在的已用区域
        int used_size = curr->size;
        plat_printf("Used:%d b, ", used_size);

        // 末尾可能存在的未用区域
        int free_size = curr_blk_tail_free(curr);
        plat_printf("Free:%d b, ", free_size);
        plat_printf("\n");

        // 检查当前包的大小是否与计算的一致
        int blk_total = pre_size + used_size + free_size;
        if (blk_total != PKTBUF_BLK_SIZE) {
            dbg_error(DBG_BUF,"bad block size. %d != %d", blk_total, PKTBUF_BLK_SIZE);
        }

        // 累计总的大小
        total_size += used_size;
    }

    // 检查总的大小是否一致
    if (total_size != buf->total_size) {
        dbg_error(DBG_BUF,"bad buf size. %d != %d", total_size, buf->total_size);
    }
}
#else
#define display_check_buf(buf)
#endif

/**
 * 释放一个block
 */
static void pktblock_free (pktblk_t * block) {
    mblock_free(&block_list, block);
}

/**
 * 回收block链
 */
static void pktblock_free_list(pktblk_t* first) {
    while (first) {
        // 先取下一个
        pktblk_t* next_block = pktbuf_blk_next(first);

        // 然后释放
        mblock_free(&block_list, first);

        // 然后调整当前的处理对像
        first = next_block;
    }
}

/**
 * @brief 释放数据包
 */
void pktbuf_free (pktbuf_t * buf) {
    pktblock_free_list(pktbuf_first_blk(buf));
    mblock_free(&pktbuf_list, buf);
}

/**
 * @brief 分配一个空闲的block
 */
static pktblk_t* pktblock_alloc(void) {
    // 不等待分配，因为会在中断中调用
    nlocker_lock(&locker);
    pktblk_t* block = mblock_alloc(&block_list, -1);
    nlocker_unlock(&locker);

    if (block) {
        block->size = 0;
        block->data = (uint8_t *)0;
        nlist_node_init(&block->node);
    }

    return block;
}

/**
 * @brief 分配一个缓冲链
 *
 * 由于分配是以BUF块为单位，所以alloc_size的大小可能会小于实际分配的BUF块的总大小
 * 那么此时就有一部分空间未用，这部分空间可能放在链表的最开始，也可能放在链表的结尾处
 * 具体存储，取决于add_front，add_front=1，将新分配的块插入到表头.否则，插入到表尾
 * 
 */
static pktblk_t* pktblock_alloc_list(int size, int add_front) {
    pktblk_t* first_block = (pktblk_t*)0;
    pktblk_t* pre_block = (pktblk_t*)0;

    while (size) {
        // 分配一个block，大小为0
        pktblk_t* new_block = pktblock_alloc();
        if (!new_block) {
            dbg_error(DBG_BUF, "no buffer for alloc(size:%d)", size);
            if (first_block) {
                // 失败，要回收释放整个链
                pktblock_free_list(first_block);
            }
            return (pktblk_t*)0;
        }

        int curr_size = 0;
        if (add_front) {
            curr_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size;

            // 反向分配，从末端往前分配空间
            new_block->size = curr_size;
            new_block->data = new_block->payload + PKTBUF_BLK_SIZE - curr_size;
            if (first_block) {
                // 将自己加在头部
                nlist_node_set_next(&new_block->node, &first_block->node);
            }

            // 如果是反向分配，第一个包总是当前分配的包
            first_block = new_block;
        } else {
            // 如果是正向分配，第一个包是第1个分配的包
            if (!first_block) {
                first_block = new_block;
            }

            curr_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size;

            // 正向分配，从前端向末端分配空间
            new_block->size = curr_size;
            new_block->data = new_block->payload;
            if (pre_block) {
                // 将自己添加到表尾
                nlist_node_set_next(&pre_block->node, &new_block->node);
            }
        }

        size -= curr_size;
        pre_block = new_block;
    }

    return first_block;
}

/**
 * 将Block链表插入到buf中
 */
static void pktbuf_insert_blk_list(pktbuf_t * buf, pktblk_t * first_blk, int add_last) {
    if (add_last) {
        // 插入尾部
        while (first_blk) {
            // 不断从first_blk中取块，然后插入到buf的后面
            pktblk_t* next_blk = pktbuf_blk_next(first_blk);

            nlist_insert_last(&buf->blk_list, &first_blk->node);         // 插入到后面
            buf->total_size += first_blk->size;

            first_blk = next_blk;
        }
    } else {
        pktblk_t *pre = (pktblk_t*)0;

        // 逐个取头部结点依次插入
        while (first_blk) {
            pktblk_t *next = pktbuf_blk_next(first_blk);

            if (pre) {
                nlist_insert_after(&buf->blk_list, &pre->node, &first_blk->node);
            } else {
                nlist_insert_first(&buf->blk_list, &first_blk->node);
            }
            buf->total_size += first_blk->size;

            pre = first_blk;
            first_blk = next;
        };
    }
}

/**
 * @brief 分配数据包块
 */
pktbuf_t* pktbuf_alloc(int size) {
    // 分配一个结构
    nlocker_lock(&locker);
    pktbuf_t* buf = mblock_alloc(&pktbuf_list, -1);
    nlocker_unlock(&locker);
    if (!buf) {
        dbg_error(DBG_BUF, "no pktbuf");
        return (pktbuf_t*)0;
    }

    // 字段值初始化
    buf->total_size = 0;
    nlist_init(&buf->blk_list);
    nlist_node_init(&buf->node);

    // 分配块空间
    if (size) {
        pktblk_t* block = pktblock_alloc_list(size, 0);
        if (!block) {
            mblock_free(&pktbuf_list, buf);
            return (pktbuf_t*)0;
        }
        pktbuf_insert_blk_list(buf, block, 1);
    }

    // 检查一下buf的完整性和正确性
    display_check_buf(buf);
    return buf;
}

/**
 * 数据包管理初始化
 */
net_err_t pktbuf_init(void) {
    dbg_info(DBG_BUF,"init pktbuf list.");

    // 建立空闲链表. TODO：在嵌入式设备中改成不可共享
    nlocker_init(&locker, NLOCKER_THREAD);
    mblock_init(&block_list, block_buffer, sizeof(pktblk_t), PKTBUF_BLK_CNT, NLOCKER_NONE);
    mblock_init(&pktbuf_list, pktbuf_buffer, sizeof(pktbuf_t), PKTBUF_BUF_CNT, NLOCKER_NONE);
    dbg_info(DBG_BUF,"init done.");

    return NET_ERR_OK;
}
