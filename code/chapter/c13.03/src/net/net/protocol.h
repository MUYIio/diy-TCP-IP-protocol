
#ifndef PROTOCOL_H
#define PROTOCOL_H

/**
 * 常见协议类型
 */
typedef enum _protocol_t {
    NET_PROTOCOL_ARP = 0x0806,     // ARP协议
    NET_PROTOCOL_IPv4 = 0x0800,      // IP协议
}protocol_t;

#endif // NET_PROTOCOL_H
