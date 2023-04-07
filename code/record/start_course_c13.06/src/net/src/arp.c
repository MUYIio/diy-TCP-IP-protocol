#include "arp.h"
#include "dbg.h"
#include "mblock.h"
#include "tools.h"
#include "pktbuf.h"
#include "protocol.h"

static arp_entry_t cache_tbl[ARP_CACHE_SIZE];
static mblock_t cache_mblock;
static nlist_t cache_list;

static net_err_t cache_init (void) {
    nlist_init(&cache_list);

    net_err_t err = mblock_init(&cache_mblock, cache_tbl, sizeof(arp_entry_t), ARP_CACHE_SIZE, NLOCKER_NONE);
    if (err < 0) {
        return err;
    }

    return NET_ERR_OK;
}

net_err_t arp_init (void) {
    net_err_t err = cache_init();
    if (err < 0) {
        dbg_error(DBG_ARP, "arp cache init failed.");
        return err;
    }

    return NET_ERR_OK;
}

net_err_t arp_make_request(netif_t * netif, const ipaddr_t * dest) {
    pktbuf_t * buf = pktbuf_alloc(sizeof(arp_pkt_t));
    if (buf == (pktbuf_t *)0) {
        dbg_error(DBG_ARP, "alloc pktbuf failed.");
        return NET_ERR_NONE;
    }

    pktbuf_set_cont(buf, sizeof(arp_pkt_t));
    arp_pkt_t * arp_packet = (arp_pkt_t *)pktbuf_data(buf);
    arp_packet->htype = x_htons(ARP_HW_ETHER);
    arp_packet->ptype = x_htons(NET_PROTOCOL_IPv4);
    arp_packet->hwlen = ETHER_HWA_SIZE;
    arp_packet->plen = IPV4_ADDR_SIZE;
    arp_packet->opcode = x_htons(ARP_REQUEST);
    plat_memcpy(arp_packet->sender_hwaddr, netif->hwaddr.addr, ETHER_HWA_SIZE);
    ipaddr_to_buf(&netif->ipaddr, arp_packet->sender_paddr);
    plat_memset(arp_packet->target_hwaddr, 0, ETHER_HWA_SIZE);
    ipaddr_to_buf(dest, arp_packet->target_paddr);

    net_err_t err = ether_raw_out(netif, NET_PROTOCOL_ARP, ether_broadcast_addr(), buf);
    if (err < 0) {
        pktbuf_free(buf);
    }
    return err;
}

net_err_t arp_make_gratuitous (netif_t * netif) {
    dbg_info(DBG_ARP, "send an gratuitous arp...");

    return arp_make_request(netif, &netif->ipaddr);
}

net_err_t arp_make_reply (netif_t * netif, pktbuf_t * buf) {
    arp_pkt_t * arp_packet = (arp_pkt_t *)pktbuf_data(buf);

    arp_packet->opcode = x_htons(ARP_REPLY);
    plat_memcpy(arp_packet->target_hwaddr, arp_packet->sender_hwaddr, ETHER_HWA_SIZE);
    plat_memcpy(arp_packet->target_paddr, arp_packet->sender_paddr, IPV4_ADDR_SIZE);
    plat_memcpy(arp_packet->sender_hwaddr, netif->hwaddr.addr, ETHER_HWA_SIZE);
    ipaddr_to_buf(&netif->ipaddr, arp_packet->sender_paddr);

    return ether_raw_out(netif, NET_PROTOCOL_ARP, arp_packet->target_hwaddr, buf);
}

static net_err_t is_pkt_ok (arp_pkt_t * arp_packet, uint16_t size, netif_t * netif) {
    if (size < sizeof(arp_pkt_t)) {
        dbg_warning(DBG_ARP, "packet size error");
        return NET_ERR_SIZE;
    }

    if ((x_ntohs(arp_packet->htype) != ARP_HW_ETHER)
    || (arp_packet->hwlen != ETHER_HWA_SIZE)
    || (x_htons(arp_packet->ptype) != NET_PROTOCOL_IPv4)
    || (arp_packet->plen != IPV4_ADDR_SIZE)) {
        dbg_warning(DBG_ARP, "packet incorroect");
        return NET_ERR_NOT_SUPPORT;
    }

    uint16_t opcode = x_ntohs(arp_packet->opcode);
    if ((opcode != ARP_REPLY) && (opcode != ARP_REQUEST)) {
        dbg_warning(DBG_ARP, "unknown opcode");
        return NET_ERR_NOT_SUPPORT;
    }

    return NET_ERR_OK;
}

net_err_t arp_in (netif_t * netif, pktbuf_t * buf) {
    dbg_info(DBG_ARP, "arp in");

    net_err_t err = pktbuf_set_cont(buf, sizeof(arp_pkt_t));
    if (err < 0) {
        return err;
    }

    arp_pkt_t * arp_packet = (arp_pkt_t *)pktbuf_data(buf);
    if (is_pkt_ok(arp_packet, buf->total_size, netif) != NET_ERR_OK) {
        return err;
    }

    if (x_ntohs(arp_packet->opcode) == ARP_REQUEST) {
        dbg_info(DBG_ARP, "arp request, send reply");
        return arp_make_reply(netif, buf);
    }

    pktbuf_free(buf);
    return NET_ERR_OK;
}
