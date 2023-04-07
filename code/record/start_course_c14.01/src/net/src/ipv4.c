#include "ipv4.h"
#include "dbg.h"

net_err_t ipv4_init (void) {
    dbg_info(DBG_IP, "init ip\n");

    dbg_info(DBG_IP, "done");
    return NET_ERR_OK;
}
