#ifndef NETIF_H
#define NETIF_H

#include "ipaddr.h"
#include "nlist.h"
#include "fixq.h"
#include "net_cfg.h"
#include "net_errr.h"
#include "pktbuf.h"

// ipv4: 32bit, 192.168.74.1
//
typedef struct _netif_hwaddr_t {
    uint8_t addr[NETIF_HWADDR_SIZE];
    uint8_t len;
}netif_hwaddr_t;

typedef enum _netif_type_t {
    NETIF_TYPE_NONE = 0,
    NETIF_TYPE_ETHER,
    NETIF_TYPE_LOOP,

    NETIF_TYPE_SIZE,
}netif_type_t;

struct _netif_t;
typedef struct _netif_ops_t {
    net_err_t (*open) (struct _netif_t * netif, void * data);
    void (*close) (struct _netif_t * netif);

    net_err_t (*xmit) (struct _netif_t * netif);
}netif_ops_t;

struct _netif_t;

typedef struct _link_layer_t {
    netif_type_t type;

    net_err_t (*open) (struct _netif_t * netif);
    void (*close) (struct _netif_t * netif);
    net_err_t (*in) (struct _netif_t * netif, pktbuf_t * buf);
    net_err_t (*out) (struct _netif_t * netif, ipaddr_t * dest, pktbuf_t * buf);
}link_layer_t;

// 138   74560983
// 159 -- 移动

typedef struct _netif_t {
    char name[NETIF_NAME_SIZE];
    netif_hwaddr_t hwaddr;

    ipaddr_t ipaddr;
    ipaddr_t netmask;
    ipaddr_t gateway;

    netif_type_t type;
    int mtu;

    enum {
        NETIF_CLOSED,
        NETIF_OPENED,
        NETIF_ACTIVE,
    }state;

    const netif_ops_t * ops;
    void * ops_data;

    const link_layer_t * link_layer;

    nlist_node_t node;

    fixq_t in_q;
    void * in_q_buf[NETIF_INQ_SIZE];
    fixq_t out_q;
    void * out_q_buf[NETIF_OUTQ_SIZE];
}netif_t;

net_err_t netif_init (void);
netif_t * netif_open (const char * dev_name, const netif_ops_t *ops, void * ops_data);
net_err_t netif_set_addr (netif_t * netif, ipaddr_t * ip, ipaddr_t * netmask, ipaddr_t * gateway);
net_err_t netif_set_hwaddr (netif_t * netif, const char * hwaddr, int len);
net_err_t netif_set_active (netif_t * netif);
net_err_t netif_set_deactive (netif_t * netif);
net_err_t netif_close (netif_t *netif);
void netif_set_default (netif_t * netif);

net_err_t netif_put_in(netif_t * netif, pktbuf_t * buf, int tmo);
pktbuf_t * netif_get_in(netif_t * netif, int tmo);
net_err_t netif_put_out(netif_t * netif, pktbuf_t * buf, int tmo);
pktbuf_t * netif_get_out(netif_t * netif, int tmo);

netif_t * netif_get_default (void);

net_err_t netif_out(netif_t * netif, ipaddr_t * ipaddr, pktbuf_t * buf);

net_err_t netif_register_layer(int type, const link_layer_t * layer);

#endif 