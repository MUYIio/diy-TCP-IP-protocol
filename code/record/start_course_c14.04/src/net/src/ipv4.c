#include "ipv4.h"
#include "dbg.h"
#include "pktbuf.h"
#include "tools.h"

net_err_t ipv4_init (void) {
    dbg_info(DBG_IP, "init ip\n");

    dbg_info(DBG_IP, "done");
    return NET_ERR_OK;
}

static net_err_t is_pkt_ok (ipv4_pkt_t * pkt, int size, netif_t * netif) {
    if (pkt->hdr.version != NET_VERSION_IPV4) {
        dbg_warning(DBG_IP, "invalid ip version");
        return NET_ERR_NOT_SUPPORT;
    }

    int hdr_len = ipv4_hdr_size(pkt);
    if (hdr_len < sizeof(ipv4_hdr_t)) {
        dbg_warning(DBG_IP, "ipv4 hader error");
        return NET_ERR_SIZE;
    }

    int total_size = x_ntohs(pkt->hdr.total_len);
    if ((total_size < sizeof(ipv4_hdr_t)) || (size < total_size)) {
        dbg_warning(DBG_IP, "ipv4 size error");
        return NET_ERR_SIZE;    
    }

    if (pkt->hdr.hdr_checksum) {
        uint16_t c = checksum16(pkt, hdr_len, 0, 1);
        if (c != 0) {
            dbg_warning(DBG_IP, "bad checksum");
            return NET_ERR_BROKEN;
        }
    }

    return NET_ERR_OK;
}

static void iphdr_ntohs (ipv4_pkt_t * pkt) {
    pkt->hdr.total_len = x_ntohs(pkt->hdr.total_len);
    pkt->hdr.id = x_ntohs(pkt->hdr.id);
    pkt->hdr.frag_all = x_ntohs(pkt->hdr.frag_all);
}

net_err_t ipv4_in (netif_t * netif, pktbuf_t * buf) {
    dbg_info(DBG_IP, "ip in\n");

    net_err_t err = pktbuf_set_cont(buf, sizeof(ipv4_hdr_t));
    if (err < 0) {
        dbg_error(DBG_IP, "ajust header failed, err = %d\n", err);
        return err;
    }

    ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(buf);
    if (is_pkt_ok(pkt, buf->total_size, netif) != NET_ERR_OK) {
        dbg_warning(DBG_INIT, "packet is broken.");
        return err;
    }

    iphdr_ntohs(pkt);

    err = pktbuf_resize(buf, pkt->hdr.total_len);
    if (err < 0) {
        dbg_error(DBG_IP, "ip pkt resize failed.");
        return err;
    }

    pktbuf_free(buf);
    return NET_ERR_OK;
}
