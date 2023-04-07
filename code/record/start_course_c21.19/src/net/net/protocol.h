#ifndef PROTOCOL_H
#define PROTOCOL_H

#define NET_PORT_EMPTY              0
#define NET_PORT_DYN_START         1024            // 动态端口号起始
#define NET_PORT_DYN_END           65535           // 最大端口号

typedef enum _protocol_t {
    NET_PROTOCOL_ARP = 0x0806,
    NET_PROTOCOL_IPv4 = 0x0800,
    NET_PROTOCOL_ICMPv4 = 1,
    NET_PROTOCOL_UDP = 0x11,
    NET_PROTOCOL_TCP = 0x6,
}protocol_t;

#endif
