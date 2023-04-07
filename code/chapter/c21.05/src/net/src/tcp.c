/**
 * @file tcp.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "tcp.h"
#include "tools.h"
#include "protocol.h"
#include "ipv4.h"
#include "mblock.h"
#include "dbg.h"
#include "exmsg.h"
#include "socket.h"
#include "timer.h"

static tcp_t tcp_tbl[TCP_MAX_NR];           // tcp控制块数组
static mblock_t tcp_mblock;                 // 空闲TCP控制块链表
static nlist_t tcp_list;                     // 已创建的控制块链表

#if DBG_DISP_ENABLED(DBG_TCP)
/**
 * @brief 显示TCP的状态
 */
void tcp_show_info (char * msg, tcp_t * tcp) {
    plat_printf("    local port: %u, remote port: %u\n", tcp->base.local_port, tcp->base.remote_port);
}

void tcp_display_pkt (char * msg, tcp_hdr_t * tcp_hdr, pktbuf_t * buf) {
    plat_printf("%s\n", msg);
    plat_printf("    sport: %u, dport: %u\n", tcp_hdr->sport, tcp_hdr->dport);
    plat_printf("    seq: %u, ack: %u, win: %d\n", tcp_hdr->seq, tcp_hdr->ack, tcp_hdr->win);
    plat_printf("    flags:");
    if (tcp_hdr->f_syn) {
        plat_printf(" syn");
    }
    if (tcp_hdr->f_rst) {
        plat_printf(" rst");
    }
    if (tcp_hdr->f_ack) {
        plat_printf(" ack");
    }
    if (tcp_hdr->f_psh) {
        plat_printf(" push");
    }
    if (tcp_hdr->f_fin) {
        plat_printf(" fin");
    }

    plat_printf("\n    len=%d", buf->total_size - tcp_hdr_size(tcp_hdr));
    plat_printf("\n");
}

void tcp_show_list (void) {
    plat_printf("-------- tcp list -----\n");

    nlist_node_t * node;
    nlist_for_each(node, &tcp_list) {
        tcp_t * tcp = (tcp_t *)nlist_entry(node, sock_t, node);
        tcp_show_info("", tcp);
    }
}
#endif

/**
 * @brief 分配一个TCP，先尝试分配空闲的，然后是重用处于time_wait的
 */
static tcp_t * tcp_get_free (int wait) {
    // 分配一个tcp sock结构
    tcp_t* tcp = (tcp_t*)mblock_alloc(&tcp_mblock, wait ? 0 : -1);
    if (!tcp) {
        return (tcp_t *)0;
    }

    return tcp;
}

/**
 * @brief 分配一个TCP块，该控制块用于用于主动打开或者被动打开
 * 对于处理listen状态的TCP，要避免锁住
 */
static tcp_t* tcp_alloc(int wait, int family, int protocol) {
    static const sock_ops_t tcp_ops;

    // 分配一个tcp sock结构
    tcp_t* tcp = tcp_get_free(wait);
    if (!tcp) {
        dbg_error(DBG_TCP, "no tcp sock");
        return (tcp_t*)0;
    }
    plat_memset(tcp, 0, sizeof(tcp_t));

    // 基础部分
    net_err_t err = sock_init((sock_t*)tcp, family, protocol, &tcp_ops);
    if (err < 0) {
        dbg_error(DBG_TCP, "create failed.");
        mblock_free(&tcp_mblock, tcp);
        return (tcp_t*)0;
    }

    return tcp;
}

/**
 * @brief 插入TCP块
 */
void tcp_insert (tcp_t * tcp) {
    nlist_insert_last(&tcp_list, &tcp->base.node);

    dbg_assert(tcp_list.count <= TCP_MAX_NR, "tcp free");
}

/**
 * @brief 创建一个TCP sock结构
 */
sock_t* tcp_create (int family, int protocol) {
    // 分配一个TCP块
    tcp_t* tcp = tcp_alloc(1, family, protocol);
    if (!tcp) {
        dbg_error(DBG_TCP, "alloc tcp failed.");
        return (sock_t *)0;
    }

    // 插入全局队列中, 注意此时端口号和IP地址全为0，因此不匹配任何不进行任何通信
    tcp_insert(tcp);
    return (sock_t *)tcp;
}

/**
 * TCP模块初始化
 */
net_err_t tcp_init(void) {
    dbg_info(DBG_TCP, "tcp init.");

    mblock_init(&tcp_mblock, tcp_tbl, sizeof(tcp_t), TCP_MAX_NR, NLOCKER_NONE);
    nlist_init(&tcp_list);

    dbg_info(DBG_TCP, "init done.");
    return NET_ERR_OK;
}
