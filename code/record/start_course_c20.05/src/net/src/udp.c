#include "udp.h"
#include "dbg.h"
#include "mblock.h"
#include "tools.h"
#include "socket.h"

static udp_t udp_tbl[UDP_MAX_NR];
static mblock_t udp_mblock;
static nlist_t udp_list;

net_err_t udp_init (void) {
    dbg_info(DBG_UDP, "udp init");

    nlist_init(&udp_list);
    mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), UDP_MAX_NR, NLOCKER_NONE);
    dbg_info(DBG_UDP, "done");
    return NET_ERR_OK;
}


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

    if (s->remote_port && (s->remote_port == dport)) {
        dbg_error(DBG_UDP, "dest is incorrect");
        return NET_ERR_PARAM;
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

    //

    *result_len = (ssize_t)len;
    return NET_ERR_OK;

end_send_to:
    pktbuf_free(pktbuf);
    return err;
}

sock_t * udp_create (int family, int protocol) {
    static const sock_ops_t udp_ops = {
        .setopt = sock_setopt,
        .sendto = udp_sendto,
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
    return (sock_t *)udp;
create_failed:
    sock_uninit(&udp->base);


    return (sock_t *)0;
}