#include "tcp.h"
#include "dbg.h"
#include "mblock.h"

static tcp_t tcp_tbl[TCP_MAX_NR];
static mblock_t tcp_mblock;
static nlist_t tcp_list;

#if DBG_DISP_ENABLED(DBG_TCP)

void tcp_show_info (char * msg, tcp_t * tcp) {
    plat_printf("%s\n", msg);
    plat_printf("local port: %d, remote port: %d\n", tcp->base.local_port, tcp->base.remote_port);
}


void tcp_show_pkt (char * msg, tcp_hdr_t * tcp_hdr, pktbuf_t * buf) {
    plat_printf("%s\n", msg);
    plat_printf("   sport: %u, dport: %u\n", tcp_hdr->sport, tcp_hdr->dport);
    plat_printf("   seq: %u, ack: %u, win: %u\n", tcp_hdr->seq, tcp_hdr->ack, tcp_hdr->win);
    plat_printf("   flags:");
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
        plat_printf(" psh");
    }
    if (tcp_hdr->f_fin) {
        plat_printf(" fin");
    }
    plat_printf("\n   len=%d", buf->total_size - tcp_hdr_size(tcp_hdr));
    plat_printf("\n");
}

void tcp_show_list (void) {
    plat_printf("----tcp list-----\n");

    nlist_node_t * node;
    nlist_for_each(node, &tcp_list) {
        tcp_t * tcp = (tcp_t *)nlist_entry(node, sock_t, node);
        tcp_show_info("", tcp);
    }
}
#endif

net_err_t tcp_init (void) {
    dbg_info(DBG_TCP, "tcp init.");

    nlist_init(&tcp_list);
    mblock_init(&tcp_mblock, tcp_tbl, sizeof(tcp_t), TCP_MAX_NR, NLOCKER_NONE);


    dbg_info(DBG_TCP, "init done.");
    return NET_ERR_OK;
}

static tcp_t * tcp_get_free (int wait) {
    tcp_t * tcp = mblock_alloc(&tcp_mblock, wait ? 0 : -1);
    if (!tcp) {
        dbg_error(DBG_UDP, "no tcp sock");
        return (tcp_t *)0;
    }

    return tcp;
}

net_err_t tcp_connect (struct _sock_t * s, const struct x_sockaddr * addr, x_socklen_t addr_len) {
    return NET_ERR_OK;
}

net_err_t tcp_close (struct _sock_t * s) {
    return NET_ERR_OK;
}

static tcp_t * tcp_alloc (int wait, int family, int protocol) {
    static const sock_ops_t tcp_ops = {
        .connect = tcp_connect,
        .close = tcp_close,
    };
    
    tcp_t * tcp = tcp_get_free(wait);
    if (!tcp) {
        dbg_error(DBG_TCP, "no tcp sock");
        return (tcp_t *)0;
    }

    plat_memset(tcp, 0, sizeof(tcp_t));

    net_err_t err = sock_init((sock_t *)tcp, family, protocol, &tcp_ops);
    if (err < 0) {
        dbg_error(DBG_TCP, "sock init failed.");
        mblock_free(&tcp_mblock, tcp);
        return (tcp_t *)0;
    }

    return tcp;
}

static void tcp_insert (tcp_t * tcp) {
    nlist_insert_last(&tcp_list, &tcp->base.node);

    dbg_assert(tcp_list.count <= TCP_MAX_NR, "tcp count err");
}

sock_t * tcp_create (int family, int protocol) {
    tcp_t * tcp = tcp_alloc(1, family, protocol);
    if (!tcp) {
        dbg_error(DBG_TCP, "alloc tcp failed.");
        return (sock_t *)0;
    }

    tcp_insert(tcp);
    return (sock_t *)tcp;
}
