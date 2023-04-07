#include "net_plat.h"

#include "dbg.h"
net_err_t net_plat_init (void) {
    dbg_info(DBG_PLAT, "init plat...");
    dbg_info(DBG_PLAT, "init done");
    return NET_ERR_OK;

}
