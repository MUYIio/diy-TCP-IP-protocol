#include "raw.h"
#include "dbg.h"
#include "mblock.h"
#include "ipv4.h"
#include "socket.h"
#include "sock.h"

static raw_t raw_tbl[RAW_MAX_NR];
static mblock_t raw_mblock;
static nlist_t raw_list;

#if DBG_DISP_ENABLED(DBG_RAW)
static void display_raw_list (void) {
    plat_printf("------- raw list ---------\n");

    nlist_node_t * node;
    int idx = 0;
    nlist_for_each(node, &raw_list) {
        raw_t * raw = (raw_t *)nlist_entry(node, sock_t, node);
        plat_printf("[%d]:\n", idx++);
        dbg_dump_ip("   local:", &raw->base.local_ip);
        dbg_dump_ip("   remote:", &raw->base.remote_ip);
        plat_printf("\n");
    }
}
#else
#define display_raw_list()
#endif

net_err_t raw_init (void) {
    dbg_info(DBG_RAW, "raw init");

    nlist_init(&raw_list);
    mblock_init(&raw_mblock, raw_tbl, sizeof(raw_t), RAW_MAX_NR, NLOCKER_NONE);
    dbg_info(DBG_RAW, "done");
    return NET_ERR_OK;
}

static net_err_t raw_sendto (struct _sock_t * s, const void * buf, size_t len, int flags, 
            const struct x_sockaddr * dest, x_socklen_t dest_len, ssize_t * result_len) {
    ipaddr_t dest_ip;
    struct x_sockaddr_in * addr = (struct x_sockaddr_in *)dest;
    ipaddr_from_buf(&dest_ip, addr->sin_addr.addr_array);
    if (!ipaddr_is_any(&s->remote_ip) && !ipaddr_is_equal(&dest_ip, &s->remote_ip)) {
        dbg_error(DBG_RAW, "dest is incorrect");
        return NET_ERR_PARAM;
    }

    pktbuf_t * pktbuf = pktbuf_alloc((int)len);
    if (!pktbuf) {
        dbg_error(DBG_SOCKET, "no buffer");
        return NET_ERR_MEM;
    }

    net_err_t err = pktbuf_write(pktbuf, (uint8_t *)buf, (int)len);
    if (err < 0) {
        dbg_error(DBG_RAW, "copy data error");
        goto end_send_to;
    }

    err = ipv4_out(s->protocol, &dest_ip, &s->local_ip, pktbuf);
    if (err < 0) {
        dbg_error(DBG_RAW, "send eroror");
        goto end_send_to;
    }

    *result_len = (ssize_t)len;
    return NET_ERR_OK;

end_send_to:
    pktbuf_free(pktbuf);
    return err;
}
static net_err_t raw_recvfrom (struct _sock_t * s, void * buf, size_t len, int flags, 
            const struct x_sockaddr * src, x_socklen_t * src_len, ssize_t * result_len) {

    raw_t * raw = (raw_t *)s;

    nlist_node_t * first = nlist_remove_first(&raw->recv_list);
    if (!first) {
        *result_len = 0;
        return NET_ERR_NEED_WAIT;
    }

    pktbuf_t * pktbuf = nlist_entry(first, pktbuf_t, node);
    ipv4_hdr_t * iphdr = (ipv4_hdr_t *)pktbuf_data(pktbuf);

    struct x_sockaddr_in * addr = (struct x_sockaddr_in *)src;
    plat_memset(addr, 0, sizeof(struct x_sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = 0;
    plat_memcpy(&addr->sin_addr, iphdr->src_ip, IPV4_ADDR_SIZE);

    int size = (pktbuf->total_size > (int)len) ? (int)len : pktbuf->total_size;
    pktbuf_reset_acc(pktbuf);

    net_err_t err = pktbuf_read(pktbuf, buf, size);
    if (err < 0) {
        pktbuf_free(pktbuf);
        dbg_error(DBG_RAW, "pktbuf read error");
        return err;
    }

    pktbuf_free(pktbuf);
    *result_len = size;
    return NET_ERR_OK;
}

net_err_t raw_close (sock_t * sock) {
    raw_t * raw = (raw_t *)sock;

    nlist_remove(&raw_list, &sock->node);

    nlist_node_t * node;
    while ((node = nlist_remove_first(&raw->recv_list))) {
        pktbuf_t * buf = nlist_entry(node, pktbuf_t, node);
        pktbuf_free(buf);
    }

    sock_uninit(sock);

    mblock_free(&raw_mblock, sock);
    display_raw_list();
    return NET_ERR_OK;
}

net_err_t raw_connect (struct _sock_t * s, const struct x_sockaddr * addr, x_socklen_t addr_len) {
    sock_connect(s, addr, addr_len);
    display_raw_list();
    return NET_ERR_OK;
}

net_err_t raw_bind (struct _sock_t * s, const struct x_sockaddr * addr, x_socklen_t addr_len) {
    net_err_t err = sock_bind(s, addr, addr_len);
    display_raw_list();
    return err;
}

sock_t * raw_create (int family, int protocol) {
    static const sock_ops_t raw_ops = {
        .sendto = raw_sendto,
        .recvfrom = raw_recvfrom,
        .setopt = sock_setopt,
        .close = raw_close,
        .send = sock_send,
        .recv = sock_recv,
        .connect = raw_connect,
        .bind = raw_bind,
    };

    raw_t * raw = mblock_alloc(&raw_mblock, -1);
    if (!raw) {
        dbg_error(DBG_RAW, "no raw sock");
        return (sock_t *)0;
    }

    net_err_t err = sock_init((sock_t *)raw, family, protocol, &raw_ops);
    if (err < 0) {
        dbg_error(DBG_RAW, "create raw failed.");
        mblock_free(&raw_mblock, raw);
        return (sock_t *)0;
    }

    nlist_init(&raw->recv_list);

    raw->base.rcv_wait = &raw->recv_wait;
    if (sock_wait_init(raw->base.rcv_wait) < 0) {
        dbg_error(DBG_RAW, "create rcv wait faile");
        goto create_failed;
    }

    nlist_insert_last(&raw_list, &raw->base.node);
    display_raw_list();
    return (sock_t *)raw;
create_failed:
    sock_uninit(&raw->base);


    return (sock_t *)0;
}

static raw_t * raw_find (ipaddr_t * src, ipaddr_t * dest, int protocol) {
    nlist_node_t * node;

    nlist_for_each(node, &raw_list) {
        raw_t * raw = (raw_t *)nlist_entry(node, sock_t, node);
        
        // 0 - protocol
        if (raw->base.protocol && (raw->base.protocol != protocol)) {
            continue;
        }

        if (!ipaddr_is_any(&raw->base.remote_ip) && !ipaddr_is_equal(&raw->base.remote_ip, src)) {
            continue;
        }

        if (!ipaddr_is_any(&raw->base.local_ip) && !ipaddr_is_equal(&raw->base.local_ip, dest)) {
            continue;
        }

        return raw;
    }

    return (raw_t *)0;
}

net_err_t raw_in (pktbuf_t * pktbuf) {
    ipv4_hdr_t * iphdr = (ipv4_hdr_t *)pktbuf_data(pktbuf);

    ipaddr_t src, dest;
    ipaddr_from_buf(&dest, iphdr->dest_ip);
    ipaddr_from_buf(&src, iphdr->src_ip);

    raw_t * raw = raw_find(&src, &dest, iphdr->protocol);
    if (!raw) {
        dbg_warning(DBG_RAW, "no raw for this packet");
        return NET_ERR_UNREACH;
    }

    if (nlist_count(&raw->recv_list) < RAW_MAX_RECV) {
        nlist_insert_last(&raw->recv_list, &pktbuf->node);

        sock_wakeup((sock_t *)raw, SOCK_WAIT_READ, NET_ERR_OK);
    } else {
        pktbuf_free(pktbuf);
    }

    return NET_ERR_OK;
}
