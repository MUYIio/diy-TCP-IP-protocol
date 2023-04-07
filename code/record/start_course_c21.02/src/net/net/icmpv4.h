#ifndef ICMPV4_H
#define ICMPV4_H

#include "netif.h"
#include "pktbuf.h"

typedef enum _icmp_type_t {
    ICMPv4_ECHO_REQUEST = 8,
    ICMPv4_ECHO_REPLY = 0,
    ICMPv4_UNREACH = 3,
}icmp_type_t;   

typedef enum _icmp_code_t {
    ICMPv4_ECHO = 0,
    ICMPv4_UNREACH_PORT = 3,
}icmp_code_t;

#pragma pack(1)
typedef struct _icmpv4_hdr_t {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
}icmpv4_hdr_t;

typedef struct _icmpv4_pkt_t {
    icmpv4_hdr_t hdr;
    union {
        uint32_t reverse;
    };
    uint8_t data[1];
}icmpv4_pkt_t;

#pragma pack()

net_err_t icmpv4_init (void);
net_err_t icmpv4_in (ipaddr_t * src_ip, ipaddr_t * netif_ip, pktbuf_t * buf);
net_err_t icmpv4_out_unreach(ipaddr_t * dest_ip, ipaddr_t * src, uint8_t code, pktbuf_t * ip_buf);

#endif 
