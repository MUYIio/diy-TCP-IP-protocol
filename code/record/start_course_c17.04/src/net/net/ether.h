#ifndef ETHER_H
#define ETHER_H

#include "net_errr.h"
#include <stdint.h>
#include "netif.h"

#define ETHER_HWA_SIZE      6
#define ETHER_MTU           1500
#define ETHER_DATA_MIN      46

#pragma pack(1)
typedef struct _ether_hdr_t {
    uint8_t dest[ETHER_HWA_SIZE];
    uint8_t src[ETHER_HWA_SIZE];
    uint16_t protocol;
}ether_hdr_t;

typedef struct _ether_pkt_t {
    ether_hdr_t hdr;
    uint8_t data[ETHER_MTU];
}ether_pkt_t;
#pragma pack()   

net_err_t ether_init (void);

const uint8_t * ether_broadcast_addr (void);
net_err_t ether_raw_out (netif_t * netif, uint16_t protocol, const uint8_t * dest, pktbuf_t * buf);
#endif 
