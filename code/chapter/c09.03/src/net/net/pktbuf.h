/**
 * 网络数据包分配管理实现
 *
 * 从网络上接收到的数据帧是变长的，用户要发送的数据也是变长的。所以需要
 * 一种灵活的方式来适应这种变长，以避免需要使用一个超大的内存块来保存数据，减少空间的浪费
 * 这里提供的是一种基于链式的存储方式，将原本需要一大块内存存储的数据，分块存储在多个数据块中
 * 数据量少时，需要的块就少；数据量大时，需要的块就多；提高了存储利用率。
 */
#ifndef PKTBUF_H
#define PKTBUF_H

#include <stdint.h>
#include "net_cfg.h"
#include "nlist.h"
#include "net_err.h"

/**
 * 数据包块类型
 */
typedef struct _pktblk_t {
    nlist_node_t node;                       // 用于连接下一个块的结
    int size;                               // 块的大小
    uint8_t* data;                          // 当前读写位置
    uint8_t payload[PKTBUF_BLK_SIZE];       // 数据缓冲区
}pktblk_t;


/**
 * 网络包类型
 */
typedef struct _pktbuf_t {
    int total_size;                         // 包的总大小
    nlist_t blk_list;                        // 包块链表
    nlist_node_t node;                       // 用于连接下一个兄弟包
}pktbuf_t;

/**
 * 获取当前block的下一个子包
 */
static inline pktblk_t * pktbuf_blk_next(pktblk_t *blk) {
    nlist_node_t * next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

// 块管理相关
net_err_t pktbuf_init(void);
pktbuf_t* pktbuf_alloc(int size);
void pktbuf_free (pktbuf_t * buf);

#endif // PKTBUF_H
