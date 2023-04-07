#ifndef PKTBUF_H
#define PKTBUF_H

#include <stdint.h>
#include "nlist.h"
#include "net_cfg.h"
#include "net_errr.h"

typedef struct _pktblk_t {
    nlist_node_t node;
    int size;
    uint8_t * data;
    uint8_t payload[PKTBUF_BLK_SIZE];
}pktblk_t;

typedef struct _pktbuf_t {
    int total_size;
    nlist_t blk_list;
    nlist_node_t node;
}pktbuf_t;

net_err_t pktbuf_init (void);
pktbuf_t *pktbuf_alloc (int size);
void pktbuf_free (pktbuf_t * buf);

static inline pktblk_t * pktblk_blk_next (pktblk_t * blk) {
    nlist_node_t * next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

static inline pktblk_t * pktbuf_first_blk (pktbuf_t * buf) {
    nlist_node_t * first = nlist_first(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
}

#endif 