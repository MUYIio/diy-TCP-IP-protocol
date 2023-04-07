#include "tcp.h"
#include "dbg.h"
#include "mblock.h"
#include "socket.h"
#include "tools.h"
#include "protocol.h"
#include "tcp_out.h"
#include "tcp_state.h"
#include "sock.h"

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

// 1-1023 -- 1024
// 1024, 1025, 1026,...2000, 
int tcp_alloc_port (void) {
#if 1
    srand((unsigned int)time(NULL));
    int search_idx = rand() % 1000 + NET_PORT_DYN_START;
#else
    static int search_idx = NET_PORT_DYN_START;
#endif
    for (int i = NET_PORT_DYN_START; i < NET_PORT_DYN_END; i++) {
        nlist_node_t * node;
        nlist_for_each(node, &tcp_list) {
            sock_t * sock = nlist_entry(node, sock_t, node);
            if (sock->local_port == search_idx) {
                break;
            }
        }

        if (++search_idx >= NET_PORT_DYN_END) {
            search_idx = NET_PORT_DYN_START;
        }

        if (!node) {
            return search_idx;
        }
    }

    return -1;
}

static uint32_t tcp_get_iss (void) {
    static uint32_t seq = 0;

    return ++seq;
}

static net_err_t tcp_init_connect (tcp_t * tcp) {
    rentry_t * rt = rt_find(&tcp->base.remote_ip);
    if (rt->netif->mtu == 0) {
        tcp->mss = TCP_DEFAULT_MSS;
    } else if (!ipaddr_is_any(&rt->next_hop)) {
        tcp->mss =  TCP_DEFAULT_MSS;
    } else {
        tcp->mss = rt->netif->mtu - sizeof(ipv4_hdr_t) - sizeof(tcp_hdr_t);
    }
    
    tcp_buf_init(&tcp->snd.buf, tcp->snd.data, TCP_SBUF_SIZE);

    tcp->snd.iss = tcp_get_iss();
    tcp->snd.una = tcp->snd.nxt = tcp->snd.iss;

    tcp->rcv.nxt = 0;
    return NET_ERR_OK;
}

net_err_t tcp_connect (struct _sock_t * s, const struct x_sockaddr * addr, x_socklen_t addr_len) {
    tcp_t * tcp = (tcp_t *)s;
    const struct x_sockaddr_in * addr_in = (const struct x_sockaddr_in *)addr;

    if (tcp->state != TCP_STATE_CLOSED) {
        dbg_error(DBG_TCP, "tcp is not closed.");
        return NET_ERR_STATE;
    }

    ipaddr_from_buf(&s->remote_ip, (uint8_t *)&addr_in->sin_addr.s_addr);
    s->remote_port = x_ntohs(addr_in->sin_port);

    if (s->local_port == NET_PORT_EMPTY) {
        int port = tcp_alloc_port();
        if (port == -1) {
            dbg_error(DBG_TCP, "alloc port failed.");
            return NET_ERR_NONE;
        }

        s->local_port = port;
    }

    if (ipaddr_is_any(&s->local_ip)) {
        rentry_t * rt = rt_find(&s->remote_ip);
        if (rt == (rentry_t *)0) {
            dbg_error(DBG_TCP, "no route to host");
            return NET_ERR_UNREACH;
        }

        ipaddr_copy(&s->local_ip, &rt->netif->ipaddr);
    }

    net_err_t err;
    if ((err = tcp_init_connect(tcp)) < 0) {
        dbg_error(DBG_TCP, "init conn failed.");
        return err;
    }

    if ((err = tcp_send_syn(tcp)) < 0) {
        dbg_error(DBG_TCP, "send syn failed.");
        return err;
    }

    tcp_set_state(tcp, TCP_STATE_SYN_SENT);
    return NET_ERR_NEED_WAIT;
}

void tcp_free (tcp_t * tcp) {
    sock_wait_destory(&tcp->conn.wait);
    sock_wait_destory(&tcp->snd.wait);
    sock_wait_destory(&tcp->rcv.wait);

    tcp->state = TCP_STATE_FREE;
    nlist_remove(&tcp_list, &tcp->base.node);
    mblock_free(&tcp_mblock, tcp);
}

net_err_t tcp_close (struct _sock_t * s) {
    tcp_t * tcp = (tcp_t *)s;

    switch (tcp->state) {
    case TCP_STATE_CLOSED:
        dbg_info(DBG_TCP, "tcp already closed.");
        tcp_free(tcp);
        return NET_ERR_OK;
    case TCP_STATE_SYN_SENT:
    case TCP_STATE_SYN_RECVD:
        tcp_abort(tcp, NET_ERR_CLOSE);
        tcp_free(tcp);
        return NET_ERR_OK;
    case TCP_STATE_CLOSE_WAIT:
        tcp_send_fin(tcp);
        tcp_set_state(tcp, TCP_STATE_LAST_ACK);
        return NET_ERR_NEED_WAIT;
    case TCP_STATE_ESTABLISHED:
        tcp_send_fin(tcp);
        tcp_set_state(tcp, TCP_STATE_FIN_WAIT_1);
        return NET_ERR_NEED_WAIT;
    default:
        dbg_error(DBG_TCP, "tcp state error.");
        return NET_ERR_STATE;
    }
    return NET_ERR_OK;
}

static net_err_t tcp_send (struct _sock_t * s, const void * buf, size_t len, int flags, ssize_t * result_len) {
    tcp_t * tcp = (tcp_t *)s;

    switch (tcp->state) {
    case TCP_STATE_CLOSED:
        dbg_error(DBG_TCP, "tcp closed.");
        return NET_ERR_CLOSE;
    case TCP_STATE_FIN_WAIT_1:
    case TCP_STATE_FIN_WAIT_2:
    case TCP_STATE_TIME_WAIT:
    case TCP_STATE_LAST_ACK:
    case TCP_STATE_CLOSING:
        dbg_error(DBG_TCP, "tcp closed.");
        return NET_ERR_CLOSE;
    case TCP_STATE_CLOSE_WAIT:
    case TCP_STATE_ESTABLISHED:
        break;
    case TCP_STATE_LISTEN:
    case TCP_STATE_SYN_RECVD:
    case TCP_STATE_SYN_SENT:
    default:
        dbg_error(DBG_TCP, "tcp state error");
        return NET_ERR_STATE;
    }

    int size = tcp_write_sndbuf(tcp, (uint8_t *)buf, (int)len);
    if (size <= 0) {
        *result_len = 0;
        return NET_ERR_NEED_WAIT;
    } else {
        *result_len = size;

        tcp_transmit(tcp);
        return NET_ERR_OK;
    }
}

static tcp_t * tcp_alloc (int wait, int family, int protocol) {
    static const sock_ops_t tcp_ops = {
        .connect = tcp_connect,
        .close = tcp_close,
        .send = tcp_send,
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

    tcp->state = TCP_STATE_CLOSED;

    if (sock_wait_init(&tcp->snd.wait) < 0) {
        dbg_error(DBG_TCP, "create snd.wait failed.");
        goto alloc_failed;
    }
    tcp->base.snd_wait = &tcp->snd.wait;

    if (sock_wait_init(&tcp->rcv.wait) < 0) {
        dbg_error(DBG_TCP, "create rcv.wait failed.");
        goto alloc_failed;
    }
    tcp->base.rcv_wait = &tcp->rcv.wait;

    if (sock_wait_init(&tcp->conn.wait) < 0) {
        dbg_error(DBG_TCP, "create conn.wait failed.");
        goto alloc_failed;
    }
    tcp->base.conn_wait = &tcp->conn.wait;
    
    return tcp;
alloc_failed:
    if (tcp->base.snd_wait) {
        sock_wait_destory(tcp->base.snd_wait);
    }
    if (tcp->base.rcv_wait) {
        sock_wait_destory(tcp->base.rcv_wait);
    }
    if (tcp->base.conn_wait) {
        sock_wait_destory(tcp->base.conn_wait);
    }
    mblock_free(&tcp_mblock, tcp);
    return (tcp_t *)0;
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

tcp_t * tcp_find(ipaddr_t * local_ip, uint16_t local_port, ipaddr_t * remote_ip, uint16_t remote_port) {
    tcp_t * match = (tcp_t *)0;

    nlist_node_t * node;

    nlist_for_each(node, &tcp_list) {
        sock_t * s = nlist_entry(node, sock_t, node);

        // 
        if ((s->local_port == local_port) && ipaddr_is_equal(&s->remote_ip, remote_ip) && (s->remote_port = remote_port)) {
            if (ipaddr_is_any(&s->local_ip)) {
                return (tcp_t *)s;
            } else if (ipaddr_is_equal(&s->local_ip, local_ip)) {
                return (tcp_t *)s;
            } else {
                
            }
        }

        // 
    }

    return match;
}

net_err_t tcp_abort (tcp_t * tcp, net_err_t err) {
    tcp_set_state(tcp, TCP_STATE_CLOSED);
    sock_wakeup(&tcp->base, SOCK_WAIT_ALL, err);
    return NET_ERR_OK;
}
