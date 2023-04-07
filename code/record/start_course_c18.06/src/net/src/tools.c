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

// blk0 - sum, checksum16(sum)  -- blkn - 
uint16_t checksum16 (int offset, void * buf, uint16_t len, uint32_t pre_sum, int complement) {
    uint16_t * curr_buf = (uint16_t *)buf;
    uint32_t checksum = pre_sum;

    if (offset & 0x1) {
        // checksum += *curr_buf++ << 8;
        uint8_t * buf = (uint8_t *)curr_buf;
        checksum += *buf++ << 8;
        curr_buf = (uint16_t *)buf;
        len--;
    }

    while (len > 1) {
        checksum += *curr_buf++;
        len -= 2;
    }

    if (len > 0) {
        checksum += *(uint8_t *)curr_buf;
    }

    uint16_t high;
    while ((high = checksum >> 16) !=0) {
        checksum = high + (checksum & 0xFFFF);
    }
    
    return complement ? (uint16_t)~checksum : (uint16_t)checksum;
}
