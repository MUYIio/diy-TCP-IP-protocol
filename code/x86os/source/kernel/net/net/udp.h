/**
 * 本源码配套的课程为 - 自己动手写TCIP/IP协议栈 源码
 * 作者：李述铜
 * 课程网址：http://01ketang.cc
 * 版权声明：本源码非开源，二次开发，或其它商用前请联系作者。
 * 注：源码不断升级中，该版本可能非最新版。如需获取最新版，请联系作者。
 */
#ifndef UDP_H
#define UDP_H

#include "sock.h"
#include "ipaddr.h"

typedef struct _udp_from_t {
    ipaddr_t from;
    uint16_t port;
}udp_from_t;

/**
 * UDP数据包头
 */
#pragma pack(1)

typedef struct _udp_hdr_t {
    uint16_t src_port;          // 源端口
    uint16_t dest_port;		    // 目标端口
    uint16_t total_len;	        // 整个数据包的长度
    uint16_t checksum;		    // 整个数据包的校验和
}udp_hdr_t;

/**
 * UDP数据包
 */
typedef struct _udp_pkt_t {
    udp_hdr_t hdr;             // UDP头部
    uint8_t data[1];            // UDP数据区
}udp_pkt_t;

#pragma pack()

/**
 * UDP控制块
 */
typedef struct _udp_t {
    sock_t  base;                   // 基础控制块
    
    // 没有用定长消息队列，目的是减少开销。定长消息队列要求有缓存表
    // 再者还会创建多个信号量等结构，资源开锁大一些
    nlist_t recv_list;           	// 接收队列
    sock_wait_t rcv_wait;           // 接收等待结构
}udp_t;

net_err_t udp_init(void);
sock_t* udp_create(int family, int protocol);
net_err_t udp_in(pktbuf_t* buf, ipaddr_t* src_ip, ipaddr_t* dest_ip);
net_err_t udp_out(ipaddr_t* dest, uint16_t dport, ipaddr_t* src, uint16_t sport, pktbuf_t* buf);
void udp_send_all (udp_t * udp);

net_err_t udp_connect(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);
net_err_t udp_sendto (struct _sock_t * sock, const void* buf, size_t len, int flags, const struct x_sockaddr* dest, 
            x_socklen_t dest_len, ssize_t * result_len);
net_err_t udp_recvfrom(sock_t* sock, void* buf, size_t len, int flags,
        struct x_sockaddr* src, x_socklen_t* addr_len, ssize_t * result_len);
net_err_t udp_bind(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);

#endif // UDP_H
