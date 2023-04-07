
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
 * @brief 分配一个空闲的block
 */
static pktblk_t* pktblock_alloc(void) {
    // 不等待分配，因为会在中断中调用
    pktblk_t* block = mblock_alloc(&block_list, -1);

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
                //pktblock_free_list(first_block);
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
 * @brief 分配数据包块
 */
pktbuf_t* pktbuf_alloc(int size) {
    // 测试代码
    pktblock_alloc_list(size, 0);
    pktblock_alloc_list(size, 1);

    return (pktbuf_t *)0;
}

/**
 * @brief 释放数据包
 */
void pktbuf_free (pktbuf_t * buf) {
}

/**
 * 数据包管理初始化
 */
net_err_t pktbuf_init(void) {
    dbg_info(DBG_BUF,"init pktbuf list.");

    // 建立空闲链表. TODO：在嵌入式设备中改成不可共享
    nlocker_init(&locker, NLOCKER_THREAD);
    mblock_init(&block_list, block_buffer, sizeof(pktblk_t), PKTBUF_BLK_CNT, NLOCKER_THREAD);
    mblock_init(&pktbuf_list, pktbuf_buffer, sizeof(pktbuf_t), PKTBUF_BUF_CNT, NLOCKER_THREAD);
    dbg_info(DBG_BUF,"init done.");

    return NET_ERR_OK;
}
