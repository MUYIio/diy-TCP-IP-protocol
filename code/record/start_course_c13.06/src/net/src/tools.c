#include "tools.h"
#include "dbg.h"

static int is_litte_endian(void) {
    uint16_t v = 0x1234;
    return *(uint8_t *)&v == 0x34;
}

net_err_t tools_init (void) {
    dbg_info(DBG_TOOLS, "init tools.");

    if (is_litte_endian() != NET_ENDIAN_LITTLE) {
        dbg_error(DBG_TOOLS, "check endian failed");
        return NET_ERR_SYS;
    }
    dbg_info(DBG_TOOLS, "done");
    return NET_ERR_OK;
}
