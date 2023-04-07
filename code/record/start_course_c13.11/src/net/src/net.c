#include "net.h"
#include "exmsg.h"
#include "net_plat.h"
#include "pktbuf.h"
#include "dbg.h"
#include "netif.h"
#include "loop.h"
#include "ether.h"
#include "tools.h"
#include "timer.h"
#include "arp.h"


net_err_t net_init (void) {
    dbg_info(DBG_INIT, "init net");

    net_plat_init();

    tools_init();
    
    exmsg_init();
    pktbuf_init();
    netif_init();

    net_timer_init();
    
    loop_init();
    ether_init();
    arp_init();
    
    return NET_ERR_OK;
}

net_err_t net_start (void) {
    exmsg_start();
    dbg_info(DBG_INIT, "net is running");
    return NET_ERR_OK;
}