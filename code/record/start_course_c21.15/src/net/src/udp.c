#include "udp.h"
#include "dbg.h"
#include "mblock.h"
#include "tools.h"
#include "socket.h"
#include "protocol.h"

static udp_t udp_tbl[UDP_MAX_NR];
static mblock_t udp_mblock;
static nlist_t udp_list;

#if DBG_DISP_ENABLED(DBG_UDP)
static void display_udp_packet (udp_pkt_t * pkt) {
    plat_printf("udp packet:\n");
    plat_printf("    sport: %d\n", pkt->hdr.src_port);
    plat_printf("    dport: %d\n", pkt->hdr.dest_port);
    plat_printf("    len: %d\n", pkt->hdr.total_len);
    plat_printf("    checksum: %d\n", pkt->hdr.checksum);
}
static void display_udp_list (void) {
    plat_printf("------- udp list ---------\n");

    nlist_node_t * node;
    int idx = 0;
    nlist_for_each(node, &udp_list) {
        udp_t * udp = (udp_t *)nlist_entry(node, sock_t, node);
        plat_printf("[%d]:\n", idx++);
        dbg_dump_ip("   local:", &udp->base.local_ip);
        plat_printf("   local port: %d, ", udp->base.local_port);
        dbg_dump_ip("   remote:", &udp->base.remote_ip);
        plat_printf("   remote port: %d, ", udp->base.remote_port);
        plat_printf("\n");
    }
}
#else
#define display_udp_packet(packet)
#define display_udp_list()
#endif

net_err_t udp_init (void) {
    dbg_info(DBG_UDP, "udp init");

    nlist_init(&udp_list);
    mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), UDP_MAX_NR, NLOCKER_NONE);
    dbg_info(DBG_UDP, "done");
    return NET_ERR_OK;
}

static int is_port_used (int port) {
    nlist_node_t * node;
    nlist_for_each(node, &udp_list) {
        sock_t * sock = (sock_t *)nlist_entry(node, sock_t, node);
        if (sock->local_port == port) {
            return 1;
        }
    }

    return 0;
}

static net_err_t alloc_port (sock_t * sock) {
    static int search_index = NET_PORT_DYN_START;

    // 1024.... 1224
    for (int i = NET_PORT_DYN_START; i < NET_PORT_DYN_END; i++) {
        int port = search_index++;
        if (search_index > NET_PORT_DYN_END) {
            search_index = NET_PORT_DYN_START;
        }

        if (!is_port_used(port)) {
            sock->local_port = port;
            return NET_ERR_OK;
        }
    }

    return NET_ERR_NONE;
}

net_err_t udp_bind (struct _sock_t * s, const struct x_sockaddr * addr, x_socklen_t addr_len) {
    const struct x_sockaddr_in * addr_in = (const struct x_sockaddr_in *)addr;

    if (s->local_port != 0) {   // ipaddr = 0
        dbg_error(DBG_UDP, "already binded");
        return NET_ERR_BINDED;
    }

    int port = x_ntohs(addr_in->sin_port);
    ipaddr_t local_ip;
    ipaddr_from_buf(&local_ip, (uint8_t *)addr_in->sin_addr.addr_array);

    // s -> local_ip, local_port <= local_ip, local_port
    // udp_list -- udp 
    nlist_node_t * node;
    udp_t * udp = (udp_t *)0;
    nlist_for_each(node, &udp_list) {
        udp_t * u = (udp_t *)nlist_entry(node, sock_t, node);
        if ((sock_t *)u == s) {
            continue;
        }

        // 127.0.0.1 2000 -- app1
        // 192.168.74.3 2000 -- app2
        if ((s->local_port == port) && (ipaddr_is_equal(&s->local_ip, &local_ip))) {
            udp = u;
            break;
        }
    }

    if (udp) {
        dbg_error(DBG_UDP, "port already used!");
        return NET_ERR_BINDED;
    } else {
        sock_bind(s, addr, addr_len);
    }

    display_udp_list();
    return NET_ERR_OK;
}

// udp_send()
static net_err_t udp_sendto (struct _sock_t * s, const void * buf, size_t len, int flags, 
            const struct x_sockaddr * dest, x_socklen_t dest_len, ssize_t * result_len) {
    ipaddr_t dest_ip;
    struct x_sockaddr_in * addr = (struct x_sockaddr_in *)dest;
    ipaddr_from_buf(&dest_ip, addr->sin_addr.addr_array);
    uint16_t dport = x_ntohs(addr->sin_port);
    if (!ipaddr_is_any(&s->remote_ip) && !ipaddr_is_equal(&dest_ip, &s->remote_ip)) {
        dbg_error(DBG_UDP, "dest is incorrect");
        return NET_ERR_PARAM;
    }

    if (s->remote_port && (s->remote_port != dport)) {
        dbg_error(DBG_UDP, "dest is incorrect");
        return NET_ERR_PARAM;
    }

    if (!s->local_port && ((s->err = alloc_port(s)) < 0)) {
        dbg_error(DBG_UDP, "no port avaliable");
        return NET_ERR_NONE;
    }

    pktbuf_t * pktbuf = pktbuf_alloc((int)len);
    if (!pktbuf) {
        dbg_error(DBG_SOCKET, "no buffer");
        return NET_ERR_MEM;
    }

    net_err_t err = pktbuf_write(pktbuf, (uint8_t *)buf, (int)len);
    if (err < 0) {
        dbg_error(DBG_UDP, "copy data error");
        goto end_send_to;
    }

    err = udp_out(&dest_ip, dport, &s->local_ip, s->local_port, pktbuf);
    if (err < 0) {
        dbg_error(DBG_UDP, "send error");
        goto end_send_to;
    }

    *result_len = (ssize_t)len;
    return NET_ERR_OK;

end_send_to:
    pktbuf_free(pktbuf);
    return err;
}

static net_err_t udp_recvfrom (struct _sock_t * s, void * buf, size_t len, int flags, 
            struct x_sockaddr * src, x_socklen_t * src_len, ssize_t * result_len) {

    udp_t * udp = (udp_t *)s;

    nlist_node_t * first = nlist_remove_first(&udp->recv_list);
    if (!first) {
        *result_len = 0;
        return NET_ERR_NEED_WAIT;
    }

    pktbuf_t * pktbuf = nlist_entry(first, pktbuf_t, node);
    udp_from_t * from = (udp_from_t *)pktbuf_data(pktbuf);
    struct x_sockaddr_in * addr = (struct x_sockaddr_in *)src;
    plat_memset(addr, 0, sizeof(struct x_sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = x_htons(from->port);
    ipaddr_to_buf(&from->from, addr->sin_addr.addr_array);

    pktbuf_remove_header(pktbuf, sizeof(udp_from_t));

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


net_err_t udp_close (sock_t * sock) {
    udp_t * udp = (udp_t *)sock;

    nlist_remove(&udp_list, &sock->node);

    nlist_node_t * node;
    while ((node = nlist_remove_first(&udp->recv_list))) {
        pktbuf_t * buf = nlist_entry(node, pktbuf_t, node);
        pktbuf_free(buf);
    }

    sock_uninit(sock);

    mblock_free(&udp_mblock, sock);
    display_udp_list();
    return NET_ERR_OK;
}

net_err_t udp_connect (struct _sock_t * s, const struct x_sockaddr * addr, x_socklen_t addr_len) {
    sock_connect(s, addr, addr_len);
    display_udp_list();
    return NET_ERR_OK;
}

sock_t * udp_create (int family, int protocol) {
    static const sock_ops_t udp_ops = {
        .setopt = sock_setopt,
        .send = sock_send,
        .sendto = udp_sendto,
        .recvfrom = udp_recvfrom,
        .recv = sock_recv,
        .close = udp_close,
        .connect = udp_connect,
        .bind = udp_bind,
    };

    udp_t * udp = mblock_alloc(&udp_mblock, -1);
    if (!udp) {
        dbg_error(DBG_UDP, "no udp sock");
        return (sock_t *)0;
    }

    net_err_t err = sock_init((sock_t *)udp, family, protocol, &udp_ops);
    if (err < 0) {
        dbg_error(DBG_UDP, "create udp failed.");
        mblock_free(&udp_mblock, udp);
        return (sock_t *)0;
    }

    nlist_init(&udp->recv_list);

    udp->base.rcv_wait = &udp->recv_wait;
    if (sock_wait_init(udp->base.rcv_wait) < 0) {
        dbg_error(DBG_UDP, "create rcv wait faile");
        goto create_failed;
    }

    nlist_insert_last(&udp_list, &udp->base.node);
    display_udp_list();
    return (sock_t *)udp;
create_failed:
    sock_uninit(&udp->base);


    return (sock_t *)0;
}

net_err_t udp_out (ipaddr_t * dest, uint16_t dport, ipaddr_t * src, uint16_t sport, pktbuf_t * buf) {
    if (ipaddr_is_any(src)) {
        rentry_t * rt = rt_find(dest);
        if (rt == (rentry_t *)0) {
            dbg_error(DBG_UDP, "no route");
            return NET_ERR_UNREACH;
        }

        src = &rt->netif->ipaddr;
    }

    net_err_t err = pktbuf_add_header(buf, sizeof(udp_hdr_t), 1);
    if (err < 0) {
        dbg_error(DBG_UDP, "add header failed");
        return NET_ERR_SIZE;
    }

    udp_hdr_t * udp_hdr = (udp_hdr_t *)pktbuf_data(buf);
    udp_hdr->src_port = x_htons(sport);
    udp_hdr->dest_port = x_htons(dport);
    udp_hdr->total_len = x_htons(buf->total_size);
    udp_hdr->checksum = 0;

    udp_hdr->checksum = checksum_peso(src->a_addr, dest->a_addr, NET_PROTOCOL_UDP, buf);

    err = ipv4_out(NET_PROTOCOL_UDP, dest, src, buf);
    if (err < 0) {
        dbg_error(DBG_UDP, "udp out err");
        return err;
    }

    return NET_ERR_OK;
}

static udp_t * udp_find (ipaddr_t * src_ip, uint16_t sport, ipaddr_t * dest_ip, uint16_t dport) {
    if (!dport) {
        return (udp_t *)0;
    }

    // 74.2 2000,   74.3 1000

    // local ip, port,  remote ip port
    // 0.0 2000,   0.0  0 --- 不限制任意计算上的应用发来的包，不限制从哪个网卡接收到
    // 74.2 2000,   0.0  0 --- 不限制任意计算上的应用发来的包，限制接受的网卡
    // 74.2 2000,   74.3 1000 -- 指定的计算机、指定端口，本地限制
    nlist_node_t * node;
    nlist_for_each(node, &udp_list) {
        sock_t * s = nlist_entry(node, sock_t, node);

        if (s->local_port != dport) {
            continue;
        }

        if (!ipaddr_is_any(&s->local_ip) && !ipaddr_is_equal(dest_ip, &s->local_ip)) {
            continue;
        }

        if (!ipaddr_is_any(&s->remote_ip) && !ipaddr_is_equal(src_ip, &s->remote_ip)) {
            continue;
        }

        if (s->remote_port && (s->remote_port != sport)) {
            continue;
        }

        return (udp_t *)s;
    }

    return (udp_t *)0;
}

static net_err_t is_pkt_ok (udp_pkt_t * pkt, int size) {
    if ((size < sizeof(udp_hdr_t)) && (size < pkt->hdr.total_len)) {
        dbg_warning(DBG_UDP, "udp packet size error");
        return NET_ERR_SIZE;
    }

    return NET_ERR_OK;
}

net_err_t udp_in (pktbuf_t * buf, ipaddr_t * src_ip, ipaddr_t * dest_ip) {
    int iphdr_size = ipv4_hdr_size((ipv4_pkt_t *)pktbuf_data(buf));

    net_err_t err = pktbuf_set_cont(buf, sizeof(udp_hdr_t) + iphdr_size);
    if (err < 0) {
        dbg_error(DBG_UDP, "set udp cont failed.");
        return err;
    }

    udp_pkt_t * udp_pkt = (udp_pkt_t *)((pktbuf_data(buf) + iphdr_size));
    uint16_t local_port = x_ntohs(udp_pkt->hdr.dest_port);
    uint16_t remote_port = x_ntohs(udp_pkt->hdr.src_port);

    udp_t * udp = (udp_t *)udp_find(src_ip, remote_port, dest_ip, local_port);
    if (!udp) {
        dbg_error(DBG_UDP, "no udp for packet");
        return NET_ERR_UNREACH;
    }

    pktbuf_remove_header(buf, iphdr_size);
    udp_pkt = (udp_pkt_t *)pktbuf_data(buf);
    if (udp_pkt->hdr.checksum) {
        pktbuf_reset_acc(buf);
        if (checksum_peso(src_ip->a_addr, dest_ip->a_addr, NET_PROTOCOL_UDP, buf)) {
            dbg_warning(DBG_UDP, "udp check sum failed.");
            return NET_ERR_BROKEN;
        }
    }

    udp_pkt->hdr.src_port = x_ntohs(udp_pkt->hdr.src_port);
    udp_pkt->hdr.dest_port = x_ntohs(udp_pkt->hdr.dest_port);
    udp_pkt->hdr.total_len = x_ntohs(udp_pkt->hdr.total_len);
    if ((err = is_pkt_ok(udp_pkt, buf->total_size)) < 0) {
        dbg_warning(DBG_UDP, "udp packet error");
        return err;
    }

    // 0.0  1000    74.3/1000  74.5/2000
    pktbuf_remove_header(buf, sizeof(udp_hdr_t) - sizeof(udp_from_t));
    udp_from_t * from = (udp_from_t *)pktbuf_data(buf);
    from->port = remote_port;
    ipaddr_copy(&from->from, src_ip);
    if (nlist_count(&udp->recv_list) < UDP_MAX_RECV) {
        nlist_insert_last(&udp->recv_list, &buf->node);

        sock_wakeup((sock_t *)udp, SOCK_WAIT_READ, NET_ERR_OK);
    } else {
        pktbuf_free(buf);
    }    
    // from(ip, port) -- data

    return NET_ERR_OK;

}
