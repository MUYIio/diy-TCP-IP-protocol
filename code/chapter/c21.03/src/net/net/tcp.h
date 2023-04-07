#ifndef TCP_H
#define TCP_H

#include "sock.h"
#include "timer.h"
#include "exmsg.h"
#include "dbg.h"
#include "net_cfg.h"

#pragma pack(1)

/**
 * TCP数据包头结构
 */
typedef struct _tcp_hdr_t {
    uint16_t sport;             // 源端口
    uint16_t dport;             // 目的端口

    // 全双工通信
    uint32_t seq;             // 自己的序列号
    uint32_t ack;             // 发给对方响应序列号

    union {
        uint16_t flags;
#if NET_ENDIAN_LITTLE
        struct {
            uint16_t resv : 4;          // 保留
            uint16_t shdr : 4;          // 头部长度
            uint16_t f_fin : 1;           // 已经完成了向对方发送数据，结束整个发送
            uint16_t f_syn : 1;           // 同步，用于初始一个连接的同步序列号
            uint16_t f_rst : 1;           // 重置连接
            uint16_t f_psh : 1;           // 推送：接收方应尽快将数据传递给应用程序
            uint16_t f_ack : 1;           // 确认号字段有效
            uint16_t f_urg : 1;           // 紧急指针有效
            uint16_t f_ece : 1;           // ECN回显：发送方接收到了一个更早的拥塞通告
            uint16_t f_cwr : 1;           // 拥塞窗口减，发送方降低其发送速率
        };
#else
        struct {
            uint16_t shdr : 4;          // 头部长度
            uint16_t resv : 4;          // 保留
            uint16_t f_cwr : 1;           // 拥塞窗口减，发送方降低其发送速率
            uint16_t f_ece : 1;           // ECN回显：发送方接收到了一个更早的拥塞通告
            uint16_t f_urg : 1;           // 紧急指针有效
            uint16_t f_ack : 1;           // 确认号字段有效
            uint16_t f_psh : 1;           // 推送：接收方应尽快将数据传递给应用程序
            uint16_t f_rst : 1;           // 重置连接
            uint16_t f_syn : 1;           // 同步，用于初始一个连接的同步序列号
            uint16_t f_fin : 1;           // 已经完成了向对方发送数据，结束整个发送
        };
#endif
    };
    uint16_t win;                       // 窗口大小，实现流量控制, 窗口缩放选项可以提供更大值的支持
    uint16_t checksum;                  // 校验和
    uint16_t urgptr;                    // 紧急指针
}tcp_hdr_t;

/**
 * TCP数据包
 */
typedef struct _tcp_pkt_t {
    tcp_hdr_t hdr;
    uint8_t data[1];
}tcp_pkt_t;

#pragma pack()

/**
 * TCP连接块
 */
typedef struct _tcp_t {
    sock_t base;            // 基础sock结构
}tcp_t;

/**
 * @brief 获取包头大小
 */
static inline int tcp_hdr_size (tcp_hdr_t * hdr) {
    return hdr->shdr * 4;
}

net_err_t tcp_init(void);

sock_t* tcp_create (int family, int protocol);

#endif // TCP_H

