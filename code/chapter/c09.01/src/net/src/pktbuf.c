
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
