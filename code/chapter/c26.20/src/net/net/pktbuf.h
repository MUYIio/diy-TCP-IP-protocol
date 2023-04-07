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

    // 读写相关
    int ref;                                // 引用计数
    int pos;                                // 当前位置总的偏移量
    pktblk_t* curr_blk;                     // 当前指向的buf
    uint8_t* blk_offset;                    // 在当前buf中的偏移量
}pktbuf_t;

/**
 * 获取当前block的下一个子包
 */
static inline pktblk_t * pktbuf_blk_next(pktblk_t *blk) {
    nlist_node_t * next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

/**
 * 取buf的第一个数据块
 * @param buf 数据缓存buf
 * @return 第一个数据块
 */
static inline pktblk_t * pktbuf_first_blk(pktbuf_t * buf) {
    nlist_node_t * first = nlist_first(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

/**
 * 取buf的最后一个数据块
 * @param buf buf 数据缓存buf
 * @return 最后一个数据块
 */
static inline pktblk_t * pktbuf_last_blk(pktbuf_t * buf) {
    nlist_node_t * first = nlist_last(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

static int inline pktbuf_total (pktbuf_t * buf) {
    return buf->total_size;
}

/**
 * 返回buf的数据区起始指针
 */
static inline uint8_t * pktbuf_data (pktbuf_t * buf) {
    pktblk_t * first = pktbuf_first_blk(buf);
    return first ? first->data : (uint8_t *)0;
}

// 块管理相关
net_err_t pktbuf_init(void);
pktbuf_t* pktbuf_alloc(int size);
void pktbuf_free (pktbuf_t * buf);
net_err_t pktbuf_add_header(pktbuf_t* buf, int size, int cont);
net_err_t pktbuf_remove_header(pktbuf_t* buf, int size);
net_err_t pktbuf_resize(pktbuf_t * buf, int to_size);
net_err_t pktbuf_join(pktbuf_t* dest, pktbuf_t* src);
net_err_t pktbuf_set_cont(pktbuf_t* buf, int size);

void pktbuf_reset_acc(pktbuf_t* buf);
int pktbuf_write(pktbuf_t* buf, uint8_t* src, int size);
int pktbuf_read(pktbuf_t* buf, uint8_t* dest, int size);
net_err_t pktbuf_seek(pktbuf_t* buf, int offset);
net_err_t pktbuf_copy(pktbuf_t* dest, pktbuf_t* src, int size);
net_err_t pktbuf_fill(pktbuf_t* buf, uint8_t v, int size);
void pktbuf_inc_ref (pktbuf_t * buf);
uint16_t pktbuf_checksum16(pktbuf_t* buf, int size, uint32_t pre_sum, int complement);

#endif // PKTBUF_H
