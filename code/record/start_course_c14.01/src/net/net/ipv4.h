#ifndef IPV4_H
#define IPV4_H

#include "net_errr.h"

#define IPV4_ADDR_SIZE          4

#pragma pack(1)

typedef struct _ipv4_hdr_t {
    uint16_t shdr_all;
    uint16_t total_len;
    uint16_t id;
    uint16_t frag_all;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t hdr_checksum;
    uint8_t src_ip[IPV4_ADDR_SIZE];
    uint8_t dest_ip[IPV4_ADDR_SIZE];
}ipv4_hdr_t;

#pragma pack()

net_err_t ipv4_init (void);

#endif 
