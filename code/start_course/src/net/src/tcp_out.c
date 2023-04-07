#include "tcp_out.h"
#include "pktbuf.h"
#include "dbg.h"
#include "tools.h"
#include "protocol.h"
#include "ipv4.h"
#include "tcp.h"

static net_err_t send_out (tcp_hdr_t * out, pktbuf_t * buf, ipaddr_t * dest, ipaddr_t * src) {
    tcp_show_pkt("tcp out", out, buf);
    
    out->sport = x_htons(out->sport);
    out->dport = x_htons(out->dport);
    out->seq = x_htonl(out->seq);
    out->ack = x_htonl(out->ack);
    out->win = x_htons(out->win);
    out->urgptr = x_htons(out->urgptr);

    out->checksum = 0;
    out->checksum = checksum_peso(dest->a_addr, src->a_addr, NET_PROTOCOL_TCP, buf);

    net_err_t err = ipv4_out(NET_PROTOCOL_TCP, dest, src, buf);
    if (err < 0) {
        dbg_warning(DBG_TCP, "send tcp error");
        pktbuf_free(buf);
        return err;
    }

    return NET_ERR_OK;
}

net_err_t tcp_send_reset (tcp_seg_t * seg) {
    tcp_hdr_t * in = seg->hdr;

    pktbuf_t * buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_warning(DBG_TCP, "no pktbuf");
        return NET_ERR_NONE;
    }

    tcp_hdr_t * out = (tcp_hdr_t *)pktbuf_data(buf);
    out->sport = in->dport;
    out->dport = in->sport;
    out->flag = 0;
    out->f_rst = 1;
    tcp_set_hdr_size(out, sizeof(tcp_hdr_t));

    if (in->f_ack) {
        out->seq = in->ack;

        out->ack = 0;
        out->f_ack = 0;
    } else {
        // out->seq ??
        out->ack = in->seq + seg->seq_len;
        out->f_ack = 1;
    }

    out->win = out->urgptr = 0;
    return send_out(out, buf, &seg->remote_ip, &seg->local_ip);
}

static int copy_send_data (tcp_t * tcp, pktbuf_t * buf, int doff, int dlen) {
    if (dlen == 0) {
        return 0;
    }

    net_err_t err = pktbuf_resize(buf, (int)buf->total_size + dlen);
    if (err < 0) {
        dbg_error(DBG_TCP, "pktbuf resize error");
        return -1;
    }

    int hdr_size = tcp_hdr_size((tcp_hdr_t *)pktbuf_data(buf));
    pktbuf_reset_acc(buf);
    pktbuf_seek(buf, hdr_size);

    tcp_buf_read_send(&tcp->snd.buf, doff, buf, dlen);
    return dlen;
}

static void get_send_info (tcp_t * tcp, int * doff, int * dlen) {
    *doff = tcp->snd.nxt - tcp->snd.una;
    *dlen = tcp_buf_cnt(&tcp->snd.buf) - *doff;

    *dlen = (*dlen > tcp->mss) ? tcp->mss : *dlen;
}

net_err_t tcp_transmit (tcp_t * tcp) {
    pktbuf_t * buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_OK;
    }

    tcp_hdr_t * hdr = (tcp_hdr_t *)pktbuf_data(buf);
    plat_memset(hdr, 0, sizeof(tcp_hdr_t));
    hdr->sport = tcp->base.local_port;
    hdr->dport = tcp->base.remote_port;
    hdr->seq = tcp->snd.nxt;
    hdr->ack = tcp->rcv.nxt;        // 自己期望接收报文序号
    hdr->flag = 0;
    hdr->f_syn = tcp->flags.syn_out;
    hdr->f_ack = tcp->flags.irs_valid;
    hdr->f_fin = tcp->flags.fin_out;
    hdr->win = 1024;
    hdr->urgptr = 0;
    tcp_set_hdr_size(hdr, sizeof(tcp_hdr_t));

    int dlen, doff;
    get_send_info(tcp, &doff, &dlen);
    if (dlen < 0) {
        return NET_ERR_OK;
    }

    copy_send_data(tcp, buf, doff, dlen);

    tcp->snd.nxt += hdr->f_syn + hdr->f_fin + dlen;

    return send_out(hdr, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

net_err_t tcp_send_syn (tcp_t * tcp) {
    tcp->flags.syn_out = 1;
    tcp_transmit(tcp);
    return NET_ERR_OK;
}

net_err_t tcp_ack_process (tcp_t * tcp, tcp_seg_t * seg) {
    tcp_hdr_t * tcp_hdr = seg->hdr;

    if (tcp->flags.syn_out) {
        tcp->snd.una++;
        tcp->flags.syn_out = 0;
    }

    int acked_cnt = tcp_hdr->ack - tcp->snd.una;
    int unacked = tcp->snd.nxt -  tcp->snd.una;
    int curr_acked = (acked_cnt > unacked) ? unacked : acked_cnt;
    if (curr_acked > 0) {
        sock_wakeup(&tcp->base, SOCK_WAIT_WRITE, NET_ERR_OK);

        tcp->snd.una += curr_acked;

        // curr_acked > tcp_buf --> FIN
        curr_acked -= tcp_buf_remove(&tcp->snd.buf, curr_acked);

        if (tcp->flags.fin_out && curr_acked) {
            tcp->flags.fin_out = 0;
        }
    }

    return NET_ERR_OK;
}

net_err_t tcp_send_ack (tcp_t * tcp, tcp_seg_t * seg) {
    pktbuf_t * buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_NONE;
    }

    tcp_hdr_t * hdr = (tcp_hdr_t *)pktbuf_data(buf);
    plat_memset(hdr, 0, sizeof(tcp_hdr_t));
    hdr->sport = tcp->base.local_port;
    hdr->dport = tcp->base.remote_port;
    hdr->seq = tcp->snd.nxt;
    hdr->ack = tcp->rcv.nxt;        // 自己期望接收报文序号
    hdr->flag = 0;
    hdr->f_ack = 1;
    hdr->win = 1024;
    hdr->urgptr = 0;
    tcp_set_hdr_size(hdr, sizeof(tcp_hdr_t));

    return send_out(hdr, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

net_err_t tcp_send_fin (tcp_t * tcp) {
    tcp->flags.fin_out = 1;
    tcp_transmit(tcp);
    return NET_ERR_OK;
}

int tcp_write_sndbuf (tcp_t * tcp, const uint8_t * buf, int len) {
    int free_cnt = tcp_buf_free_cnt(&tcp->snd.buf);
    if (free_cnt <= 0) {
        return 0;
    }

    int wr_len = (len > free_cnt) ? free_cnt : len;
    tcp_buf_write_send(&tcp->snd.buf, buf, wr_len);
    return wr_len;
}
