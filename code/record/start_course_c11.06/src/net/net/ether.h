#ifndef ETHER_H
#define ETHER_H

#include "net_errr.h"
#include <stdint.h>

#define ETHER_HWA_SIZE      6
#define ETHER_MTU           1500

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

#endif 
