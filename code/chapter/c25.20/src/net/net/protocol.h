
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define NET_PORT_DYN_START         1024            // 动态端口号起始
#define NET_PORT_DYN_END           65535           // 最大端口号
/**
 * 常用端口号
 */
typedef enum _port_t {
    NET_PORT_EMPTY = 0,                // 无用端口号
}port_t;

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
