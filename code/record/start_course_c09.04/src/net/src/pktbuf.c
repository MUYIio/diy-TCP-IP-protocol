#include "pktbuf.h"
#include "dbg.h"
#include "mblock.h"
#include "nlocker.h"

static nlocker_t locker;
static pktblk_t block_buffer[PKTBUF_BLK_CNT];
static mblock_t block_list;
static pktbuf_t pktbuf_buffer[PKTBUF_BUF_CNT];
static mblock_t pktbuf_list;

static inline int curr_blk_tail_free (pktblk_t * blk) {
    return (int)((blk->payload + PKTBUF_BLK_SIZE) - (blk->data + blk->size));
}

#if DBG_DISP_ENABLED(DBG_BUF) 
static void display_check_buf (pktbuf_t * buf) {
    if (!buf) {
        dbg_error(DBG_BUF, "invalid buf, buf == 0");
        return;
    }

    plat_printf("check buf %p: size %d\n", buf, buf->total_size);
    pktblk_t * curr;
    int index = 0, total_size = 0;
    for (curr = pktbuf_first_blk(buf); curr; curr = pktblk_blk_next(curr)) {
        plat_printf("%d: ", index++);

        if ((curr->data < curr->payload) || (curr->data >= curr->payload + PKTBUF_BLK_SIZE)) {
            dbg_error(DBG_BUF, "bad block data");
        }

        int pre_size = (int)(curr->data - curr->payload);
        plat_printf("pre: %d b, ", pre_size);

        int used_size = curr->size;
        plat_printf("used: %d b, ", used_size);

        int free_size = curr_blk_tail_free(curr);
        plat_printf("free: %d b, \n", free_size);

        int blk_total = pre_size + used_size + free_size;
        if (blk_total != PKTBUF_BLK_SIZE) {
            dbg_error(DBG_BUF, "bad block size: %d != %d", blk_total, PKTBUF_BLK_SIZE);
        }

        total_size += used_size;
    }

    if (total_size != buf->total_size) {
        dbg_error(DBG_BUF, "bad buf size: %d != %d", total_size, buf->total_size);
    }
}
#else
#define display_check_buf(buf)
#endif

net_err_t pktbuf_init (void) {
    dbg_info(DBG_BUF, "init pktbuf");

    nlocker_init(&locker, NLCOKER_THREAD);
    mblock_init(&block_list, block_buffer, sizeof(pktblk_t), PKTBUF_BLK_CNT, NLCOKER_THREAD);
    mblock_init(&pktbuf_list, pktbuf_buffer, sizeof(pktbuf_t), PKTBUF_BUF_CNT, NLCOKER_THREAD);
    
    dbg_info(DBG_BUF, "init done");
    return NET_ERR_OK;
}

static pktblk_t * pktblock_alloc (void) {
    pktblk_t * block = mblock_alloc(&block_list, -1);
    if (block) {
        block->size = 0;
        block->data = (uint8_t *)0;
        nlist_node_init(&block->node);
    }

    return block;
}

static pktblk_t * pktblock_alloc_list (int size, int add_front) {
    pktblk_t * first_block = (pktblk_t *)0;
    pktblk_t * pre_block = (pktblk_t *)0;

    while (size) {
        pktblk_t * new_block = pktblock_alloc();
        if (!new_block) {
            dbg_error(DBG_BUF, "no buffer for alloc(%d)", size);

            // block_free()??
            return (pktblk_t *)0;
        }

        int curr_size = 0;
        if (add_front) {
            curr_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size;
            new_block->size = curr_size;
            new_block->data = new_block->payload + PKTBUF_BLK_SIZE - curr_size;
            if (first_block) {
                nlist_node_set_next(&new_block->node, &first_block->node);
            }

            first_block = new_block;
        } else {
            if (!first_block) {
                first_block = new_block;
            }

            curr_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size;
            new_block->size = curr_size;
            new_block->data = new_block->payload;
            if (pre_block) {
                nlist_node_set_next(&pre_block->node, &new_block->node);
            }
        }

        size -= curr_size;
        pre_block = new_block;
    }

    return first_block;
}

static void pktbuf_insert_blk_list(pktbuf_t * buf, pktblk_t * first_blk, int add_list) {
    if (add_list) {
        while (first_blk) {
            pktblk_t * next_blk = pktblk_blk_next(first_blk);

            nlist_insert_last(&buf->blk_list, &first_blk->node);
            buf->total_size += first_blk->size;

            first_blk = next_blk;
        }
    } else {
        pktblk_t * pre = (pktblk_t *)0;

        while (first_blk) {
            pktblk_t * next_blk = pktblk_blk_next(first_blk);
            if (pre) {
                nlist_insert_after(&buf->blk_list, &pre->node, &first_blk->node);
            } else {
                nlist_insert_first(&buf->blk_list, &first_blk->node);
            }

            pre = first_blk;
            first_blk = next_blk;
        }
    }
}

pktbuf_t *pktbuf_alloc (int size) {
    pktbuf_t * buf = mblock_alloc(&pktbuf_list, -1);
    if (!buf) {
        dbg_error(DBG_BUF, "no buffer");
        return (pktbuf_t *)0;
    }

    buf->total_size = 0;
    nlist_init(&buf->blk_list);
    nlist_node_init(&buf->node);

    if (size) {
        pktblk_t * block = pktblock_alloc_list(size, 1);
        if (!block) {
            mblock_free(&pktbuf_list, buf);
            return (pktbuf_t *)0;
        }

        pktbuf_insert_blk_list(buf, block, 1);
    }

    display_check_buf(buf);
    return buf;
}

void pktbuf_free (pktbuf_t * buf) {

}