#include "ether.h"
#include "netif.h"
#include "dbg.h"

static net_err_t ether_open (struct _netif_t * netif) {
    return NET_ERR_OK;
}

static void ether_close (struct _netif_t * netif) {

}

static net_err_t ether_in (struct _netif_t * netif, pktbuf_t * buf) {
    return NET_ERR_OK;
}

static net_err_t ether_out (struct _netif_t * netif, ipaddr_t * dest, pktbuf_t * buf) {
    return NET_ERR_OK;
}

net_err_t ether_init (void) {
    static const link_layer_t link_layer = {
        .type = NETIF_TYPE_ETHER,
        .open = ether_open,
        .close = ether_close,
        .in = ether_in,
        .out = ether_out
    };

    dbg_info(DBG_ETHER, "init ether");

    net_err_t err = netif_register_layer(NETIF_TYPE_ETHER, &link_layer);
    if (err < 0) {
        dbg_info(DBG_ETHER, "register error");
        return err;
    }

    dbg_info(DBG_ETHER, "done");
    return NET_ERR_OK;
}
