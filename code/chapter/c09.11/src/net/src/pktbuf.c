
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
 * 为数据包新增一个头
 */
net_err_t pktbuf_add_header(pktbuf_t* buf, int size, int cont) {
    pktblk_t * block = pktbuf_first_blk(buf);

    // 如果块前面有足够的空闲，直接分配掉就可以了
    int resv_size = (int)(block->data - block->payload);
    if (size <= resv_size) {
        block->size += size;
        block->data -= size;
        buf->total_size += size;

        display_check_buf(buf);
        return NET_ERR_OK;
    }

    // 没有足够的空间，需要额外分配块添加到头部, 第一个块即使有空间也用不了
    if (cont) {
        // 要求连续时，但大小超过1个块，否则肯定是分配不了的
        if (size > PKTBUF_BLK_SIZE) {
            dbg_error(DBG_BUF,"is_contious && size too big %d > %d", size, PKTBUF_BLK_SIZE);
            return NET_ERR_NONE;
        }

        // 不超过一个块，则需要在新分配的块中设置连续空间，原保留区空间不影响
        block = pktblock_alloc_list(size, 1);
        if (!block) {
            dbg_error(DBG_BUF,"no buffer for alloc(size:%d)", size);
            return NET_ERR_NONE;
        }
    } else {
        // 非连续分配，可能在当前buf前面还有一点空间，可以先利用起来
        block->data = block->payload;
        block->size += resv_size;
        buf->total_size += resv_size;
        size -= resv_size;

        // 再分配其它未用的空间
        block = pktblock_alloc_list(size, 1);
        if (!block) {
            dbg_error(DBG_BUF,"no buffer for alloc(size:%d)", size);
            return NET_ERR_NONE;
        }
    }

    // 加入块列表的头部
    pktbuf_insert_blk_list(buf, block, 0);
    display_check_buf(buf);
    return NET_ERR_OK;
}

/**
 * 为数据包移去一个包头
 */
net_err_t pktbuf_remove_header(pktbuf_t* buf, int size) {
    pktblk_t * block = pktbuf_first_blk(buf);
    while (size) {
        pktblk_t* next_blk = pktbuf_blk_next(block);

        // 当前包足够减去头，在当前包上操作即可完成要求的工作
        if (size < block->size) {
            block->data += size;
            block->size -= size;
            buf->total_size -= size;
            break;
        }

        // 不够，则减去当前整个包头
        int curr_size = block->size;

        // 不够，移除当前的块
        nlist_remove_first(&buf->blk_list);
        pktblock_free(block);

        size -= curr_size;
        buf->total_size -= curr_size;
        block = next_blk;
    }

    display_check_buf(buf);
    return NET_ERR_OK;
}

/**
 * 调整数据包的大小
 */
net_err_t pktbuf_resize(pktbuf_t* buf, int to_size) {
    // 大小相同，无需调整
    if (to_size == buf->total_size) {
        return NET_ERR_OK;
    }

    if (buf->total_size == 0) {
        // 原来空间为0，直接分配就行
        pktblk_t* blk = pktblock_alloc_list(to_size, 0);
        if (!blk) {
            dbg_error(DBG_BUF, "not enough blk.");
            return NET_ERR_MEM;
        }

        pktbuf_insert_blk_list(buf, blk, 1);
    } else if (to_size == 0) {
        pktblock_free_list(pktbuf_first_blk(buf));
        buf->total_size = 0;
        nlist_init(&buf->blk_list);
    }  else if (to_size > buf->total_size) {
        // 扩充尾部，整体变长. 需要知道自己最后剩余多少，如果不够，还要额外分配
        // 不考虑最后一个块头部有空间的情况，简化处理
        pktblk_t * tail_blk = pktbuf_last_blk(buf);

        // 要定位到最后，看空间是否足够
        int inc_size = to_size - buf->total_size;       // 大小的差值
        int remain_size = curr_blk_tail_free(tail_blk);
        if (remain_size >= inc_size) {
            // 足够，则使用该空间直接分配，不需要额外分配新的block
            tail_blk->size += inc_size;
            buf->total_size += inc_size;
        } else {
            // 为扩充的空间分配buf链，不算剩余空间
            pktblk_t * new_blks = pktblock_alloc_list(inc_size - remain_size, 0);
            if (!new_blks) {
                dbg_error(DBG_BUF, "not enough blk.");
                return NET_ERR_NONE;
            }

            // 连接两个buf链
            tail_blk->size += remain_size;      // 剩余空间全部利用起来
            buf->total_size += remain_size;
            pktbuf_insert_blk_list(buf, new_blks, 1);
        }
    } else {
        // 缩减尾部，整体变短
        int total_size = 0;

        // 遍历到达需要保留的最后一个缓存块
        pktblk_t* tail_blk;
        for (tail_blk = pktbuf_first_blk(buf); tail_blk; tail_blk = pktbuf_blk_next(tail_blk)) {
            total_size += tail_blk->size;
            if (total_size >= to_size) {
                break;
            }
        }

        if (tail_blk == (pktblk_t*)0) {
            return NET_ERR_SIZE;
        }

        // 减掉后续所有块链中的容量
        pktblk_t * curr_blk = pktbuf_blk_next(tail_blk);
        total_size = 0;
        while (curr_blk) {
            // 先取后续的结点
            pktblk_t * next_blk = pktbuf_blk_next(curr_blk);

            // 删除当前block
            nlist_remove(&buf->blk_list, &curr_blk->node);
            pktblock_free(curr_blk);

            total_size += curr_blk->size;
            curr_blk = next_blk;
        }

        // 调整tail_blk的大小
        tail_blk->size -= buf->total_size - total_size - to_size;
        buf->total_size = to_size;
    }

    display_check_buf(buf);
    return NET_ERR_OK;
}

/**
 * 将src拼到dest的尾部，组成一个buf链
 * 不考虑两外包连接处的包合并释放问题，以简化处理
 */
net_err_t pktbuf_join(pktbuf_t* dest, pktbuf_t* src) {
    dbg_assert(dest->ref != 0, "buf freed");
    dbg_assert(src->ref != 0, "buf freed");
    // 逐个从src的块列表中取出，然后加入到dest中
    pktblk_t* first;

    while ((first = pktbuf_first_blk(src))) {
        // 从src中移除首块
        nlist_remove_first(&src->blk_list);

        // 插入到块链表中
        pktbuf_insert_blk_list(dest, first, 1);
    }

    pktbuf_free(src);

    dbg_info(DBG_BUF,"join result:");
    display_check_buf(dest);
    return NET_ERR_OK;
}

/**
 * 将包的最开始size个字节，配置成连续空间
 * 常用于对包头进行解析时，或者有其它选项字节时
 */
net_err_t pktbuf_set_cont(pktbuf_t* buf, int size) {
    // 必须要有足够的长度
    if (size > buf->total_size) {
        dbg_error(DBG_BUF,"size(%d) > total_size(%d)", size, buf->total_size);
        return NET_ERR_SIZE;
    }

    // 超过1个POOL的大小，返回错误
    if (size > PKTBUF_BLK_SIZE) {
        dbg_error(DBG_BUF,"size too big > %d", PKTBUF_BLK_SIZE);
        return NET_ERR_SIZE;
    }

    // 已经处于连续空间，不用处理
    pktblk_t * first_blk = pktbuf_first_blk(buf);
    if (size <= first_blk->size) {
        display_check_buf(buf);
        return NET_ERR_OK;
    }

    // 先将第一个blk的数据往前挪，以在尾部腾出size空间
#if 0
    uint8_t * dest = first_blk->payload + PKTBUF_BLK_SIZE - size;
    plat_memmove(dest, first_blk->data, first_blk->size);   // 注意处理内存重叠
    first_blk->data = dest;
    dest += first_blk->size;          // 指向下一块复制的目的地
#else
    uint8_t * dest = first_blk->payload;
    for (int i = 0; i < first_blk->size; i++) {
        *dest++ = first_blk->data[i];
    }
    first_blk->data = first_blk->payload;
#endif

    // 再依次将后续的空间，搬到buf中，直到buf中的大小达到size
    int remain_size = size - first_blk->size;
    pktblk_t * curr_blk = pktbuf_blk_next(first_blk);
    while (remain_size && curr_blk) {
        // 计算本次移动的数据量
        int curr_size = (curr_blk->size > remain_size) ? remain_size : curr_blk->size;

        // 将curr中的数据，移动到buf中
        plat_memcpy(dest, curr_blk->data, curr_size);
        dest += curr_size;
        curr_blk->data += curr_size;
        curr_blk->size -= curr_size;
        first_blk->size += curr_size;
        remain_size -= curr_size;

        // 复制完后，curr_blk可能无数据，释放掉，从其后一个包继续复制
        if (curr_blk->size == 0) {
            pktblk_t* next_blk = pktbuf_blk_next(curr_blk);

            nlist_remove(&buf->blk_list, &curr_blk->node);
            pktblock_free(curr_blk);

            curr_blk = next_blk;
        }
    }

    display_check_buf(buf);
    return NET_ERR_OK;
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
