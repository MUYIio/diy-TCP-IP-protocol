#ifndef IPV4_H
#define IPV4_H

#include "net_errr.h"
#include <stdint.h>
#include "netif.h"
#include "pktbuf.h"
#include "net_cfg.h"

#define IPV4_ADDR_SIZE          4

#define NET_VERSION_IPV4        4

#pragma pack(1)

typedef struct _ipv4_hdr_t {
    union {
        struct {
#if NET_ENDIAN_LITTLE
            uint16_t shdr : 4;
            uint16_t version : 4;
            uint16_t tos : 8;
#else
            uint16_t version : 4;
            uint16_t shdr : 4;
            uint16_t tos : 8;
#endif
        };
        uint16_t shdr_all;
    };
    
    uint16_t total_len;
    uint16_t id;
    uint16_t frag_all;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t hdr_checksum;
    uint8_t src_ip[IPV4_ADDR_SIZE];     // uint32_t
    uint8_t dest_ip[IPV4_ADDR_SIZE];
}ipv4_hdr_t;

typedef struct _ipv4_pkt_t {
    ipv4_hdr_t hdr;
    uint8_t data[1];
}ipv4_pkt_t;

#pragma pack()

net_err_t ipv4_init (void);
net_err_t ipv4_in (netif_t * netif, pktbuf_t * buf);

static inline int ipv4_hdr_size (ipv4_pkt_t * pkt) {
    return pkt->hdr.shdr * 4;
}
#endif 
