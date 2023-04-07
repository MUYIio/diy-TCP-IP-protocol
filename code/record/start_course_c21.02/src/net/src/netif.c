#include "netif.h"
#include "mblock.h"
#include "dbg.h"
#include "pktbuf.h"
#include "exmsg.h"
#include "protocol.h"
#include "ether.h"
#include "ipv4.h"

static netif_t netif_buffer[NETIF_DEV_CNT];
static mblock_t netif_mblock;
static nlist_t netif_list;
static netif_t * netif_default;

static const link_layer_t * link_layers[NETIF_TYPE_SIZE];

#if DBG_DISP_ENABLED(DBG_NETIF)
void display_netif_list (void) {
    plat_printf("netif list:\n");

    nlist_node_t * node;
    nlist_for_each(node, &netif_list) {
        netif_t * netif = nlist_entry(node, netif_t, node);

        plat_printf("%s:", netif->name);
        switch (netif->state)
        {
        case NETIF_CLOSED:
            plat_printf("  %s  ", "closed");
            break;
        case NETIF_OPENED:
            plat_printf("  %s  ", "opened");
            break;        
        case NETIF_ACTIVE:
            plat_printf("  %s  ", "active");
            break;        
        default:
            break;
        }

        switch (netif->type)
        {
        case NETIF_TYPE_ETHER:
            plat_printf("  %s  ", "ether");
            break;
        case NETIF_TYPE_LOOP:
            plat_printf("  %s  ", "loop");
            break;
        default:
            break;
        }

        plat_printf(" mtu=%d \n", netif->mtu);
        dbg_dump_hwaddr("hwaddr:", netif->hwaddr.addr, netif->hwaddr.len);
        dbg_dump_ip(" ip:", &netif->ipaddr);
        dbg_dump_ip(" netmask:", &netif->netmask);
        dbg_dump_ip(" gateway:", &netif->gateway);
        plat_printf("\n");
    }
}
#else
#define display_netif_list()
#endif

net_err_t netif_init (void) {
    dbg_info(DBG_NETIF, "init netif");

    nlist_init(&netif_list);
    mblock_init(&netif_mblock, netif_buffer, sizeof(netif_t), NETIF_DEV_CNT, NLOCKER_NONE);

    netif_default = (netif_t *)0;

    plat_memset((void *)link_layers, 0, sizeof(link_layers));
    dbg_info(DBG_NETIF, "init done.\n");
    return NET_ERR_OK;
}

net_err_t netif_register_layer(int type, const link_layer_t * layer) {
    if ((type < 0) || (type >= NETIF_TYPE_SIZE)) {
        dbg_error(DBG_NETIF, "type error");
        return NET_ERR_PARAM;
    }

    if (link_layers[type]) {
        dbg_error(DBG_NETIF, "link layer exist");
        return NET_ERR_EXIST;
    }

    link_layers[type] = layer;
    return NET_ERR_OK;
}

static const link_layer_t * netif_get_layer(int type) {
    if ((type < 0) || (type >= NETIF_TYPE_SIZE)) {
        dbg_error(DBG_NETIF, "type error");
        return (const link_layer_t *)0;
    }    

    return link_layers[type];
}

netif_t * netif_open (const char * dev_name, const netif_ops_t *ops, void * ops_data) {
    netif_t * netif = (netif_t *)mblock_alloc(&netif_mblock, -1);
    if (!netif) {
        dbg_error(DBG_NETIF, "no netif");
        return (netif_t *)0;
    }

    ipaddr_set_any(&netif->ipaddr);
    ipaddr_set_any(&netif->netmask);
    ipaddr_set_any(&netif->gateway);

    plat_strncpy(netif->name, dev_name, NETIF_NAME_SIZE);
    netif->name[NETIF_NAME_SIZE - 1] = '\0';

    plat_memset(&netif->hwaddr, 0, sizeof(netif_hwaddr_t));
    netif->type = NETIF_TYPE_NONE;
    netif->mtu = 0;
    nlist_node_init(&netif->node);

    net_err_t err = fixq_init(&netif->in_q, netif->in_q_buf, NETIF_INQ_SIZE, NLCOKER_THREAD);
    if (err < 0) {
        dbg_error(DBG_NETIF, "netif in_q init failed.");
        mblock_free(&netif_mblock, netif);
        return (netif_t *)0;
    }

    err = fixq_init(&netif->out_q, netif->out_q_buf, NETIF_OUTQ_SIZE, NLCOKER_THREAD);
    if (err < 0) {
        dbg_error(DBG_NETIF, "netif out_q init failed.");
        mblock_free(&netif_mblock, netif);
        fixq_destory(&netif->in_q);
        return (netif_t *)0;
    }    

    netif->ops = ops;
    netif->ops_data = ops_data;
    err = ops->open(netif, ops_data);
    if (err < 0) {
        dbg_error(DBG_NETIF, "netif ops open errr");
        goto free_return;
    }
    netif->state = NETIF_OPENED;

    if (netif->type == NETIF_TYPE_NONE) {
        dbg_error(DBG_NETIF, "netif type unknown");
        goto free_return;
    }

    netif->link_layer = netif_get_layer(netif->type);
    if (!netif->link_layer && (netif->type != NETIF_TYPE_LOOP)) {
        dbg_error(DBG_NETIF, "no link layer, netif name: %s\n", dev_name);
        goto free_return;
    }

    nlist_insert_last(&netif_list, &netif->node);
    display_netif_list();
    return netif;
free_return:
    if (netif->state == NETIF_OPENED) {
        netif->ops->close(netif);
    }
    fixq_destory(&netif->in_q);
    fixq_destory(&netif->out_q);
    mblock_free(&netif_mblock, netif);
    return (netif_t *)0;
}

net_err_t netif_set_addr (netif_t * netif, ipaddr_t * ip, ipaddr_t * netmask, ipaddr_t * gateway) {
    ipaddr_copy(&netif->ipaddr, ip ? ip : ipaddr_get_any());
    ipaddr_copy(&netif->netmask, netmask ? netmask : ipaddr_get_any());
    ipaddr_copy(&netif->gateway, gateway ? gateway : ipaddr_get_any());
    return NET_ERR_OK;
}

net_err_t netif_set_hwaddr (netif_t * netif, const char * hwaddr, int len) {
    plat_memcpy(netif->hwaddr.addr, hwaddr, len);
    netif->hwaddr.len = len;
    return NET_ERR_OK;
}

net_err_t netif_set_active (netif_t * netif) {
    if (netif->state != NETIF_OPENED) {
        dbg_error(DBG_NETIF, "netif is not opened");
        return NET_ERR_STATE;
    }
    
    if (!netif_default && (netif->type != NETIF_TYPE_LOOP)) {
        netif_set_default(netif);
    }

    if (netif->link_layer) {
        net_err_t err = netif->link_layer->open(netif);
        if (err < 0) {
            dbg_info(DBG_NETIF, "active error");
            return err;
        }
    }

    ipaddr_t ip = ipaddr_get_net(&netif->ipaddr, &netif->netmask);
    rt_add(&ip, &netif->netmask, ipaddr_get_any(), netif);

    ipaddr_from_str(&ip, "255.255.255.255");
    rt_add(&netif->ipaddr, &ip, ipaddr_get_any(), netif);


    netif->state = NETIF_ACTIVE;
    display_netif_list();
    return NET_ERR_OK;
}

net_err_t netif_set_deactive (netif_t * netif) {
    if (netif->state != NETIF_ACTIVE) {
        dbg_error(DBG_NETIF, "netif is not actived");
        return NET_ERR_STATE;
    }

    if (netif->link_layer) {
        netif->link_layer->close(netif);
    }

    pktbuf_t * buf;
    while ((buf = fixq_recv(&netif->in_q, -1)) != (pktbuf_t *)0) {
        pktbuf_free(buf);
    }

    while ((buf = fixq_recv(&netif->out_q, -1)) != (pktbuf_t *)0) {
        pktbuf_free(buf);
    }

    if (netif_default == netif) {
        netif_default = (netif_t *)0;
        rt_remove(ipaddr_get_any(), ipaddr_get_any());
    }

    ipaddr_t ip = ipaddr_get_net(&netif->ipaddr, &netif->netmask);
    rt_remove(&ip, &netif->netmask);

    ipaddr_from_buf(&ip, "255.255.255.255");
    rt_remove(&netif->ipaddr, &ip);

    netif->state = NETIF_OPENED;
    display_netif_list();
    return NET_ERR_OK;
}

net_err_t netif_close (netif_t *netif) {
    if (netif->state == NETIF_ACTIVE) {
        dbg_error(DBG_NETIF, "netif is active");
        return NET_ERR_STATE;
    }

    netif->ops->close(netif);
    netif->state = NETIF_CLOSED;

    nlist_remove(&netif_list, &netif->node);

    mblock_free(&netif_mblock, netif);
    display_netif_list();
    return NET_ERR_OK;
}

void netif_set_default (netif_t * netif) {
    netif_default = netif;

    if (!ipaddr_is_any(&netif->gateway)) {
        if (netif_default) {
            rt_remove(ipaddr_get_any(), ipaddr_get_any());
        }
        rt_add(ipaddr_get_any(), ipaddr_get_any(), &netif->gateway, netif);
    }
}

netif_t * netif_get_default (void) {
    return netif_default;
}

net_err_t netif_put_in(netif_t * netif, pktbuf_t * buf, int tmo) {
    net_err_t err = fixq_send(&netif->in_q, buf, tmo);
    if (err < 0) {
        dbg_warning(DBG_NETIF, "netif in_q full");
        return NET_ERR_FULL;
    }

    exmsg_netif_in(netif);
    return NET_ERR_OK;
}

pktbuf_t * netif_get_in(netif_t * netif, int tmo) {
    pktbuf_t * buf = fixq_recv(&netif->in_q, tmo);
    if (buf) {
        pktbuf_reset_acc(buf);
        return buf;
    }

    dbg_info(DBG_NETIF, "netif inq_q emptry");
    return (pktbuf_t *)0;
}

net_err_t netif_put_out(netif_t * netif, pktbuf_t * buf, int tmo) {
    net_err_t err = fixq_send(&netif->out_q, buf, tmo);
    if (err < 0) {
        dbg_warning(DBG_NETIF, "netif out_q full");
        return NET_ERR_FULL;
    }

    return NET_ERR_OK;
}

pktbuf_t * netif_get_out(netif_t * netif, int tmo) {
    pktbuf_t * buf = fixq_recv(&netif->out_q, tmo);
    if (buf) {
        pktbuf_reset_acc(buf);
        return buf;
    }

    dbg_info(DBG_NETIF, "netif out_q emptry");
    return (pktbuf_t *)0;
}

net_err_t netif_out(netif_t * netif, ipaddr_t * ipaddr, pktbuf_t * buf) {
    if (netif->link_layer) {
        // netif->link_layer->out
        net_err_t err = netif->link_layer->out(netif, ipaddr, buf);
        if (err < 0) {
            dbg_warning(DBG_NETIF, "netif link out err");
            return err;
        }

        return NET_ERR_OK;
    } else {
        net_err_t err = netif_put_out(netif, buf, -1);
        if (err < 0) {
            dbg_info(DBG_NETIF, "send failed, queue full");
            return err;
        }
        return netif->ops->xmit(netif);
    }
}
