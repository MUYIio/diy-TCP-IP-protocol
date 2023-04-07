#ifndef NETIF_DISC_H
#define NETIF_DISC_H

#include "net_err.h"
#include "netif.h"

/**
 * 网络接口打开
 */
typedef struct _disc_data_t {
    const uint8_t* hwaddr;             // 网卡的mac地址
}netif_disc_data_t;

extern const netif_ops_t netif_disc_ops;

#endif
