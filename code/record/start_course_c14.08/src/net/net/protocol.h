#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum _protocol_t {
    NET_PROTOCOL_ARP = 0x0806,
    NET_PROTOCOL_IPv4 = 0x0800,
    NET_PROTOCOL_ICMPv4 = 1,
    NET_PROTOCOL_UDP = 0x11,
    NET_PROTOCOL_TCP = 0x6,
}protocol_t;

#endif
