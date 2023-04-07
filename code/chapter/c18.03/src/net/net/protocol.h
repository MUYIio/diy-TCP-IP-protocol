
#ifndef PROTOCOL_H
#define PROTOCOL_H

/**
 * 常见协议类型
 */
typedef enum _protocol_t {
    NET_PROTOCOL_ARP = 0x0806,     // ARP协议
    NET_PROTOCOL_IPv4 = 0x0800,      // IP协议
    NET_PROTOCOL_ICMPv4 = 0x1,         // ICMP协议
    NET_PROTOCOL_UDP = 0x11,          // UDP协议
    NET_PROTOCOL_TCP = 0x06,          // TCP协议
}protocol_t;

#endif // NET_PROTOCOL_H
