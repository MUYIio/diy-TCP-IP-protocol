#include "ipv4.h"
#include "dbg.h"
#include "pktbuf.h"
#include "tools.h"
#include "protocol.h"
#include "icmpv4.h"

static uint16_t pakcet_id = 0;

#if DBG_DISP_ENABLED(DBG_IP)

static void display_ip_pkt(ipv4_pkt_t * pkt) {
    ipv4_hdr_t * ip_hdr = &pkt->hdr;

    plat_printf("--------------ip ---------------\n");
    plat_printf("    version: %d\n", ip_hdr->version);
    plat_printf("    header len: %d\n", ipv4_hdr_size(pkt));
    plat_printf("    total len: %d\n", ip_hdr->total_len);
    plat_printf("    id: %d\n", ip_hdr->id);
    plat_printf("    ttl: %d\n", ip_hdr->ttl);
    plat_printf("    protocol: %d\n", ip_hdr->protocol);
    plat_printf("    checksum: %d\n", ip_hdr->hdr_checksum);
    dbg_dump_ip_buf("     src ip:", ip_hdr->src_ip);   
    dbg_dump_ip_buf(" dest ip:", ip_hdr->dest_ip);
    plat_printf("\n--------------ip end ---------------\n");
}

#else

#endif

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

static void iphdr_htons(ipv4_pkt_t * pkt) {
    pkt->hdr.total_len = x_htons(pkt->hdr.total_len);
    pkt->hdr.id = x_htons(pkt->hdr.id);
    pkt->hdr.frag_all = x_htons(pkt->hdr.frag_all);
}

static net_err_t ip_normal_in(netif_t * netif, pktbuf_t * buf, ipaddr_t * src_ip, ipaddr_t * dest_ip) {
    ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(buf);

    // 192.172.74.255
    display_ip_pkt(pkt);

    switch(pkt->hdr.protocol) {
    case NET_PROTOCOL_ICMPv4: {
        net_err_t err = icmpv4_in(src_ip, &netif->ipaddr, buf);
        if (err < 0) {
            dbg_warning(DBG_IP, "icmp in failed.");
            return err;
        }
        break;
    }
    case NET_PROTOCOL_UDP:
        iphdr_htons(pkt);
        icmpv4_out_unreach(src_ip, &netif->ipaddr, ICMPv4_UNREACH_PORT, buf);
        break;
    case NET_PROTOCOL_TCP:
        break;
    default:
        dbg_warning(DBG_IP, "unknown protocol");
        break;
    }

    return NET_ERR_UNREACH;
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

    ipaddr_t dest_ip, src_ip;
    ipaddr_from_buf(&dest_ip, pkt->hdr.dest_ip);
    ipaddr_from_buf(&src_ip, pkt->hdr.src_ip);

    // 192.168.7.2   -- 192.168.7.255 -- 255.255.255.255
    if (!ipaddr_is_match(&dest_ip, &netif->ipaddr, &netif->netmask)) {
        dbg_error(DBG_IP, "ipaddr not match");
        return NET_ERR_UNREACH;
    }

    err = ip_normal_in(netif, buf, &src_ip, &dest_ip);
    return NET_ERR_OK;
}

net_err_t ipv4_out (uint8_t protocol, ipaddr_t * dest, ipaddr_t * src, pktbuf_t * buf) {
    dbg_info(DBG_IP, "send an ip pkt");

    net_err_t err = pktbuf_add_header(buf, sizeof(ipv4_hdr_t), 1);
    if (err < 0) {
        dbg_error(DBG_IP, "add header failed");
        return NET_ERR_SIZE;
    }

    ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(buf);
    pkt->hdr.shdr_all = 0;
    pkt->hdr.version = NET_VERSION_IPV4;
    ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
    pkt->hdr.total_len = buf->total_size;
    pkt->hdr.id = pakcet_id++;
    pkt->hdr.frag_all = 0;
    pkt->hdr.ttl = NET_IP_DEFAULT_TTL;
    pkt->hdr.protocol = protocol;
    pkt->hdr.hdr_checksum = 0;
    ipaddr_to_buf(src, pkt->hdr.src_ip);
    ipaddr_to_buf(dest, pkt->hdr.dest_ip);

    iphdr_htons(pkt);
    pktbuf_reset_acc(buf);
    pkt->hdr.hdr_checksum = pktbuf_checksum16(buf, ipv4_hdr_size(pkt), 0, 1);

    display_ip_pkt(pkt);

    err = netif_out(netif_get_default(), dest, buf); 
    if (err < 0) {
        dbg_warning(DBG_IP, "send ip packet");
        return err;
    }

    return NET_ERR_OK;
}

