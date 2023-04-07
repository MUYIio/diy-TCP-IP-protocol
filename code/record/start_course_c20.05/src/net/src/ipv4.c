#include "ipv4.h"
#include "dbg.h"
#include "pktbuf.h"
#include "tools.h"
#include "protocol.h"
#include "icmpv4.h"
#include "mblock.h"
#include "timer.h"
#include "raw.h"

static uint16_t pakcet_id = 0;

static ip_frag_t frag_array[IP_FRAGS_MAX_NR];
static mblock_t frag_mblock;
static nlist_t frag_list;
static net_timer_t frag_timer;

static nlist_t rt_list;
static rentry_t rt_table[IP_RTTABLE_SIZE];
static mblock_t rt_mblock;

#if DBG_DISP_ENABLED(DBG_IP)
void rt_nlist_display (void) {
    plat_printf("rt table\n");

    nlist_node_t * node;
    nlist_for_each(node, &rt_list) {
        rentry_t * entry = nlist_entry(node, rentry_t, node);
        dbg_dump_ip("   net: ", &entry->net);
        dbg_dump_ip("   mask: ", &entry->mask);
        dbg_dump_ip("   next_hop: ", &entry->next_hop);
        plat_printf("    netif: %s\n", entry->netif->name);
    }
}

#else
#define rt_nlist_display()
#endif

void rt_init (void) {
    nlist_init(&rt_list);
    mblock_init(&rt_mblock, rt_table, sizeof(rentry_t), IP_RTTABLE_SIZE, NLOCKER_NONE);
}

void rt_add (ipaddr_t * net, ipaddr_t * mask, ipaddr_t * next_hop, netif_t * netif) {
    rentry_t * entry = (rentry_t *)mblock_alloc(&rt_mblock, -1);
    if (!entry) {
        dbg_warning(DBG_IP, "alloc rt entry failed.");
        return;
    }

    ipaddr_copy(&entry->net, net);
    ipaddr_copy(&entry->mask, mask);
    ipaddr_copy(&entry->next_hop, next_hop);
    entry->netif = netif;
    entry->mask_1_cnt = ipaddr_1_cnt(mask);

    nlist_insert_last(&rt_list, &entry->node);

    rt_nlist_display();
}

void rt_remove (ipaddr_t * net, ipaddr_t * mask) {
    nlist_node_t * node;

    nlist_for_each(node, &rt_list) {
        rentry_t * entry = nlist_entry(node, rentry_t, node);
        if (ipaddr_is_equal(&entry->net, net) && ipaddr_is_equal(&entry->mask, mask)) {
            nlist_remove(&rt_list, node);

            rt_nlist_display();
            return;
        }
    }
}

rentry_t * rt_find (ipaddr_t * ip) {
    rentry_t * e = (rentry_t *)0;
    nlist_node_t * node;

    nlist_for_each(node, &rt_list) {
        rentry_t * entry = nlist_entry(node, rentry_t, node);

        ipaddr_t net = ipaddr_get_net(ip, &entry->mask);
        if (!ipaddr_is_equal(&net, &entry->net)) {
            continue;
        }

        if (!e || (e->mask_1_cnt < entry->mask_1_cnt)) {
            e = entry;
        }
    }

    return e;
}


static int get_data_size (ipv4_pkt_t * pkt) {
    return pkt->hdr.total_len - ipv4_hdr_size(pkt);
}

static uint16_t get_frag_start (ipv4_pkt_t * pkt) {
    return pkt->hdr.frag_offset * 8;
}

static uint16_t get_frag_end (ipv4_pkt_t * pkt) {
    return get_frag_start(pkt) + get_data_size(pkt);
}

#if DBG_DISP_ENABLED(DBG_IP)

static void display_ip_frags (void) {
    plat_printf("ip frags:\n");

    int f_index = 0;
    nlist_node_t * f_node;
    nlist_for_each(f_node, &frag_list) {
        ip_frag_t * frag = nlist_entry(f_node, ip_frag_t, node);

        plat_printf("[%d]: \n", f_index++);
        dbg_dump_ip("   ip:", &frag->ip);
        plat_printf("   id: %d\n", frag->id);
        plat_printf("   tmo: %d\n", frag->tmo);
        plat_printf("   bufs: %d\n", nlist_count(&frag->buf_list));


        plat_printf("   bufs:\n");
        nlist_node_t * p_node;
        int p_index = 0;
        nlist_for_each(p_node, &frag->buf_list) {
            pktbuf_t * buf = nlist_entry(p_node, pktbuf_t, node);

            ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(buf);
            plat_printf("   B%d:[%d-%d],  ", p_index++, get_frag_start(pkt), get_frag_end(pkt) - 1);
        }
        plat_printf("\n");
    }
}

static void display_ip_pkt(ipv4_pkt_t * pkt) {
    ipv4_hdr_t * ip_hdr = &pkt->hdr;

    plat_printf("--------------ip ---------------\n");
    plat_printf("    version: %d\n", ip_hdr->version);
    plat_printf("    header len: %d\n", ipv4_hdr_size(pkt));
    plat_printf("    total len: %d\n", ip_hdr->total_len);
    plat_printf("    id: %d\n", ip_hdr->id);
    plat_printf("    ttl: %d\n", ip_hdr->ttl);
    plat_printf("    frag offset: %d\n", ip_hdr->frag_offset);
    plat_printf("    frag more: %d\n", ip_hdr->more);
    plat_printf("    protocol: %d\n", ip_hdr->protocol);
    plat_printf("    checksum: %d\n", ip_hdr->hdr_checksum);
    dbg_dump_ip_buf("     src ip:", ip_hdr->src_ip);   
    dbg_dump_ip_buf(" dest ip:", ip_hdr->dest_ip);
    plat_printf("\n--------------ip end ---------------\n");
}

#else
#define display_ip_frags()
#define display_ip_pkt(pk)

#endif

static void frag_free_buf_list (ip_frag_t * frag) {
    nlist_node_t * node;
    while ((node = nlist_remove_first(&frag->buf_list))) {
        pktbuf_t * buf = nlist_entry(node, pktbuf_t, node);
        pktbuf_free(buf);
    }
}

static ip_frag_t * frag_alloc (void) {
    ip_frag_t * frag = mblock_alloc(&frag_mblock, -1);
    if (!frag) {
        nlist_node_t * node = nlist_remove_last(&frag_list);
        frag = nlist_entry(node, ip_frag_t, node);
        if (frag) {
            frag_free_buf_list(frag);
        }
    }
    return frag;
}

static void frag_free (ip_frag_t * frag) {
    frag_free_buf_list(frag);
    nlist_remove(&frag_list, &frag->node);
    mblock_free(&frag_mblock, frag);
}

static void frag_add (ip_frag_t * frag, ipaddr_t * ip, uint16_t id) {
    ipaddr_copy(&frag->ip, ip);
    frag->tmo = IP_FRAG_TMO / IP_FRAG_SCAN_PERIOD;
    frag->id = id;
    nlist_node_init(&frag->node);
    nlist_init(&frag->buf_list);

    nlist_insert_first(&frag_list, &frag->node);
}

static ip_frag_t * frag_find (ipaddr_t * ip, uint16_t id) {
    nlist_node_t * curr;

    nlist_for_each(curr, &frag_list) {
        ip_frag_t * frag = nlist_entry(curr, ip_frag_t, node);

        if (ipaddr_is_equal(ip, &frag->ip) && (id == frag->id)) {
            nlist_remove(&frag_list, curr);
            nlist_insert_first(&frag_list, curr);
            return frag;
        }
    }

    return (ip_frag_t *)0;
}

static net_err_t frag_insert (ip_frag_t * frag, pktbuf_t * buf, ipv4_pkt_t * pkt) {
    if (nlist_count(&frag->buf_list) >= IP_FRAG_MAX_BUF_NR) {
        dbg_warning(DBG_IP, "too many bufs on frag");
        frag_free(frag);
        return NET_ERR_FULL;  
    }

    nlist_node_t * node;
    nlist_for_each(node, &frag->buf_list) {
        pktbuf_t * curr_buf = nlist_entry(node, pktbuf_t, node);
        ipv4_pkt_t * curr_pkt = (ipv4_pkt_t *)pktbuf_data(curr_buf);

        uint16_t curr_start = get_frag_start(curr_pkt);
        if (get_frag_start(pkt) == curr_start) {
            return NET_ERR_EXIST;
        } else if (get_frag_end(pkt) <= curr_start) {
            nlist_node_t * pre = nlist_node_pre(node);
            if (pre) {
                nlist_insert_after(&frag->buf_list, pre, &buf->node);
            } else {
                nlist_insert_first(&frag->buf_list, &buf->node);        // node
            }
            return NET_ERR_OK;
        }
    }

    nlist_insert_last(&frag->buf_list, &buf->node);
    return NET_ERR_OK;
}

static int frag_is_all_arrived (ip_frag_t * frag) {
    int offset = 0;

    ipv4_pkt_t * pkt = (ipv4_pkt_t *)0;
    nlist_node_t * node;
    nlist_for_each(node, &frag->buf_list) {
        pktbuf_t * buf = nlist_entry(node, pktbuf_t, node);
        pkt = (ipv4_pkt_t *)pktbuf_data(buf);

        int curr_offset = get_frag_start(pkt);
        if (curr_offset != offset) {
            return 0;
        }

        offset += get_data_size(pkt);
    }

    return pkt ? !pkt->hdr.more : 0;
}

static pktbuf_t * frag_join (ip_frag_t * frag) {
    pktbuf_t * target = (pktbuf_t *)0;
    nlist_node_t * node;
    while ((node = nlist_remove_first(&frag->buf_list))) {
        pktbuf_t * curr = nlist_entry(node, pktbuf_t, node);

        if (!target) {
            target = curr;
            continue;
        }

        ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(curr);
        net_err_t err = pktbuf_remove_header(curr, ipv4_hdr_size(pkt));
        if (err < 0) {
            dbg_error(DBG_IP, "remove hdr failed.");
            pktbuf_free(curr);
            goto free_and_return;
        }

        err = pktbuf_join(target, curr);
        if (err < 0) {
            dbg_error(DBG_IP, "join ip frag failed.");
            pktbuf_free(curr);
            goto free_and_return;
        }
    }
    frag_free(frag);
    return target;

free_and_return:
    if (target) {
        pktbuf_free(target);
    }
    frag_free(frag);
    return (pktbuf_t *)0;
}


static void frag_tmo (net_timer_t * timer, void * arg) {
    nlist_node_t * curr, * next;

    for (curr = nlist_first(&frag_list); curr; curr = next) {
        next = nlist_node_next(curr);

        ip_frag_t * frag = nlist_entry(curr, ip_frag_t, node);
        if (--frag->tmo <= 0) {
            frag_free(frag);
        } 
    }   
}

static net_err_t frag_init (void) {
    nlist_init(&frag_list);
    mblock_init(&frag_mblock, frag_array, sizeof(ip_frag_t), IP_FRAGS_MAX_NR, NLOCKER_NONE);

    net_err_t err = net_timer_add(&frag_timer, "frag timer", frag_tmo, (void *)0, 
        IP_FRAG_SCAN_PERIOD * 1000, NET_TIMER_RELOAD);
    if (err < 0) {
        dbg_error(DBG_IP, "create frag timer failed.");
        return err;
    }
    return NET_ERR_OK;
}

net_err_t ipv4_init (void) {
    dbg_info(DBG_IP, "init ip\n");

    net_err_t err = frag_init();
    if (err < 0) {
        dbg_error(DBG_IP, "frag init failed.");
        return err;
    }

    rt_init();

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
        uint16_t c = checksum16(0, pkt, hdr_len, 0, 1);
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
    default: {
        dbg_warning(DBG_IP, "unknown protocol");

        net_err_t err = raw_in(buf);
        if (err < 0) {
            dbg_warning(DBG_IP, "raw in error");
            return err;
        }
        break;
    }
    }

    return NET_ERR_UNREACH;
}

static net_err_t ip_frag_in (netif_t * netif, pktbuf_t * buf, ipaddr_t * src_ip, ipaddr_t * dest_ip) {
    ipv4_pkt_t * curr = (ipv4_pkt_t *)pktbuf_data(buf);

    ip_frag_t * frag = frag_find(src_ip, curr->hdr.id);
    if (!frag) {
        frag = frag_alloc();
        frag_add(frag, src_ip, curr->hdr.id);
    }

    net_err_t err = frag_insert(frag, buf, curr);
    if (err < 0) {
        dbg_warning(DBG_IP, "frag insert failed.");
        return err;
    }

    if (frag_is_all_arrived(frag)) {
        pktbuf_t * full_buf = frag_join(frag);
        if (!full_buf) {
            dbg_error(DBG_IP, "join ip bufs failed.");
            display_ip_frags();
            return NET_ERR_OK;
        }

        err = ip_normal_in(netif, full_buf, src_ip, dest_ip);
        if (err < 0) {
            dbg_warning(DBG_IP, "ip frag in failed.", err);
            pktbuf_free(full_buf);
            return NET_ERR_OK;
        }
    }

    display_ip_frags();
    return NET_ERR_OK;
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

    if (pkt->hdr.frag_offset || pkt->hdr.more) {
        err = ip_frag_in(netif, buf, &src_ip, &dest_ip);
    } else {
        err = ip_normal_in(netif, buf, &src_ip, &dest_ip);
    }
    return NET_ERR_OK;
}

net_err_t ip_frag_out (uint8_t protocol, ipaddr_t * dest, ipaddr_t * src, pktbuf_t * buf, ipaddr_t * next, netif_t * netif) {
    dbg_info(DBG_IP, "frag send an ip pkt");

    pktbuf_reset_acc(buf);

    int offset = 0;
    int total = buf->total_size;
    while (total) {
        int curr_size = total;
        if (curr_size + sizeof(ipv4_hdr_t) > netif->mtu) {
            curr_size = netif->mtu - sizeof(ipv4_hdr_t);
        }

        pktbuf_t * dest_buf = pktbuf_alloc(curr_size + sizeof(ipv4_hdr_t));
        if (!dest_buf) {
            dbg_error(DBG_IP, "alloc buffer for frag send failed.");
            return NET_ERR_NONE;
        }

        ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(dest_buf);
        pkt->hdr.shdr_all = 0;
        pkt->hdr.version = NET_VERSION_IPV4;
        ipv4_set_hdr_size(pkt, sizeof(ipv4_hdr_t));
        pkt->hdr.total_len = dest_buf->total_size;
        pkt->hdr.id = pakcet_id;
        pkt->hdr.frag_all = 0;
        pkt->hdr.ttl = NET_IP_DEFAULT_TTL;
        pkt->hdr.protocol = protocol;
        pkt->hdr.hdr_checksum = 0;
        if (!src || ipaddr_is_any(src)) {
            ipaddr_to_buf(&netif->ipaddr, pkt->hdr.src_ip);
        } else {
            ipaddr_to_buf(src, pkt->hdr.src_ip);
        }
        ipaddr_to_buf(dest, pkt->hdr.dest_ip);

        pkt->hdr.frag_offset = offset >> 3;
        pkt->hdr.more = total > curr_size;

        pktbuf_seek(dest_buf, sizeof(ipv4_hdr_t));
        net_err_t err = pktbuf_copy(dest_buf, buf, curr_size);
        if (err < 0) {
            dbg_error(DBG_IP, "frag copy failed.");
            pktbuf_free(dest_buf);
            return err;
        }

        pktbuf_remove_header(buf, curr_size);
        pktbuf_reset_acc(buf);

        iphdr_htons(pkt);
        pktbuf_reset_acc(dest_buf);
        pkt->hdr.hdr_checksum = pktbuf_checksum16(dest_buf, ipv4_hdr_size(pkt), 0, 1);

        display_ip_pkt(pkt);

        err = netif_out(netif, next, dest_buf); 
        if (err < 0) {
            dbg_warning(DBG_IP, "send ip packet");
            pktbuf_free(dest_buf);
            return err;
        }       

        total -= curr_size;
        offset += curr_size; 
    }

    pakcet_id++;
    pktbuf_free(buf);
    return NET_ERR_OK;
}

net_err_t ipv4_out (uint8_t protocol, ipaddr_t * dest, ipaddr_t * src, pktbuf_t * buf) {
    dbg_info(DBG_IP, "send an ip pkt");

    rentry_t * rt = rt_find(dest);
    if (!rt) {
        dbg_error(DBG_IP, "send failed. no route");
        return NET_ERR_UNREACH;
    }

    ipaddr_t next_hop;
    if (ipaddr_is_any(&rt->next_hop)) {
        ipaddr_copy(&next_hop, dest);       // 直接交付
    } else {
        ipaddr_copy(&next_hop, &rt->next_hop);
    }

    netif_t * netif = rt->netif;
    if (netif->mtu && ((buf->total_size + sizeof(ipv4_hdr_t)) > netif->mtu)) {
        net_err_t err = ip_frag_out(protocol, dest, src, buf, &next_hop, netif);
        if (err < 0) {
            dbg_warning(DBG_IP, "send ip frag failed.");
            return err;
        }

        return NET_ERR_OK;
    }

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
    if (!src || ipaddr_is_any(src)) {
        ipaddr_to_buf(&netif->ipaddr, pkt->hdr.src_ip);
    } else {
        ipaddr_to_buf(src, pkt->hdr.src_ip);
    }
    ipaddr_to_buf(dest, pkt->hdr.dest_ip);

    iphdr_htons(pkt);
    pktbuf_reset_acc(buf);
    pkt->hdr.hdr_checksum = pktbuf_checksum16(buf, ipv4_hdr_size(pkt), 0, 1);

    display_ip_pkt(pkt);

    // 8.8.8.8
    err = netif_out(netif, &next_hop, buf); 
    if (err < 0) {
        dbg_warning(DBG_IP, "send ip packet");
        return err;
    }

    return NET_ERR_OK;
}

