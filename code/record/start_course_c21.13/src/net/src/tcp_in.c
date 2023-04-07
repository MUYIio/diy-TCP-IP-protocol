#include "tcp_in.h"
#include "pktbuf.h"
#include "ipaddr.h"
#include "dbg.h"
#include "tools.h"
#include "protocol.h"
#include "tcp_out.h"
#include "tcp_state.h"

void tcp_seg_init (tcp_seg_t * seg, pktbuf_t * buf, ipaddr_t * local, ipaddr_t * remote) {
    seg->buf = buf;
    seg->hdr = (tcp_hdr_t *)pktbuf_data(buf);

    ipaddr_copy(&seg->local_ip, local);
    ipaddr_copy(&seg->remote_ip, remote);
    seg->data_len = buf->total_size - tcp_hdr_size(seg->hdr);
    seg->seq = seg->hdr->seq;
    seg->seq_len = seg->data_len + seg->hdr->f_syn + seg->hdr->f_fin;
}

net_err_t tcp_in (pktbuf_t * buf, ipaddr_t * src_ip, ipaddr_t * dest_ip) {
    static const tcp_proc_t tcp_state_proc[] = {
        [TCP_STATE_CLOSED] = tcp_closed_in,
        [TCP_STATE_LISTEN] = tcp_listen_in,
        [TCP_STATE_SYN_SENT] = tcp_syn_sent_in,
        [TCP_STATE_SYN_RECVD] = tcp_syn_recvd_in,
        [TCP_STATE_ESTABLISHED] = tcp_established_in,
        [TCP_STATE_FIN_WAIT_1] = tcp_fin_wait_1_in,
        [TCP_STATE_FIN_WAIT_2] = tcp_fin_wait_2_in,
        [TCP_STATE_CLOSING] = tcp_closing_in,
        [TCP_STATE_TIME_WAIT] = tcp_time_wait_in,
        [TCP_STATE_CLOSE_WAIT] = tcp_close_wait_in,
        [TCP_STATE_LAST_ACK] = tcp_last_ack_in,
    };

    tcp_hdr_t * tcp_hdr = (tcp_hdr_t *)pktbuf_data(buf);
    if (tcp_hdr->checksum) {
        pktbuf_reset_acc(buf);
        if (checksum_peso(src_ip->a_addr, dest_ip->a_addr, NET_PROTOCOL_TCP, buf)) {
            dbg_warning(DBG_TCP, "tcp check sum failed.");
            return NET_ERR_BROKEN;
        }
    }

    if ((buf->total_size < sizeof(tcp_hdr_t)) || (buf->total_size < tcp_hdr_size(tcp_hdr))) {
        dbg_warning(DBG_TCP, "tcp pkt size error");
        return NET_ERR_SIZE;
    }

    if (!tcp_hdr->sport || !tcp_hdr->dport) {
        dbg_warning(DBG_TCP, "port == 0");
        return NET_ERR_BROKEN;
    }

    if (tcp_hdr->flag == 0) {
        dbg_warning(DBG_TCP, "flag  == 0");
        return NET_ERR_BROKEN;
    }

    tcp_hdr->sport = x_ntohs(tcp_hdr->sport);
    tcp_hdr->dport = x_ntohs(tcp_hdr->dport);
    tcp_hdr->seq = x_ntohl(tcp_hdr->seq);
    tcp_hdr->ack = x_ntohl(tcp_hdr->ack);
    tcp_hdr->win = x_ntohs(tcp_hdr->win);
    tcp_hdr->urgptr = x_ntohs(tcp_hdr->urgptr);

    tcp_show_pkt("tcp in", tcp_hdr, buf);

    tcp_seg_t seg;
    tcp_seg_init (&seg, buf, dest_ip, src_ip);

    tcp_t * tcp = tcp_find(dest_ip, tcp_hdr->dport, src_ip, tcp_hdr->sport);
    if (!tcp) {
        dbg_info(DBG_TCP, "no tcp found");
        tcp_send_reset(&seg);
        pktbuf_free(buf);

        tcp_show_list();
        return NET_ERR_OK;
    }

    tcp_state_proc[tcp->state](tcp, &seg);
    tcp_show_info("after tcp in", tcp);

    //tcp_show_list();
    return NET_ERR_OK;
}
