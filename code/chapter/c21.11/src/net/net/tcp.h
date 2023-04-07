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
 * TCP状态机中的各种状态
 */
typedef enum _tcp_state_t {
    TCP_STATE_FREE = 0,             // 空闲状态，非标准状态的一部分
    TCP_STATE_CLOSED,
    TCP_STATE_LISTEN,
    TCP_STATE_SYN_SENT,
    TCP_STATE_SYN_RECVD,
    TCP_STATE_ESTABLISHED,
    TCP_STATE_FIN_WAIT_1,
    TCP_STATE_FIN_WAIT_2,
    TCP_STATE_CLOSING,
    TCP_STATE_TIME_WAIT,
    TCP_STATE_CLOSE_WAIT,
    TCP_STATE_LAST_ACK,

    TCP_STATE_MAX,
}tcp_state_t;

/**
 * @brief TCP报文段结构
 */
typedef struct _tcp_seg_t {
    ipaddr_t local_ip;               // 本地IP
    ipaddr_t remote_ip;              // 远端IP
    tcp_hdr_t * hdr;                // TCP包
    pktbuf_t * buf;                 // Buffer包
    uint32_t data_len;              // 数据长度
    uint32_t seq;                   // 起始序号
    uint32_t seq_len;               // 序列号空间长度
}tcp_seg_t;

/**
 * TCP连接块
 */
typedef struct _tcp_t {
    sock_t base;            // 基础sock结构
    struct {
        uint32_t syn_out : 1;        // 输出SYN标志位
    }flags;

    tcp_state_t state;      // TCP状态

    struct {
        sock_wait_t wait;       // 连接等待结构
    }conn;

    struct {
        uint32_t una;	    // 已发但未确认区域的起始序号
        uint32_t nxt;	    // 未发送的起始序号
        uint32_t iss;	    // 起始发送序号
        sock_wait_t wait;   // 发送等待结构
    }snd;

    struct {
        uint32_t nxt;	    // 下一个期望接受的序号
        uint32_t iss;	    // 起始接收序号
        sock_wait_t wait;   // 接收等待结构
    }rcv;
}tcp_t;

/**
 * @brief 设置包头大小
 */
static inline void tcp_set_hdr_size (tcp_hdr_t * hdr, int size) {
    hdr->shdr = size / 4;
}

/**
 * @brief 获取包头大小
 */
static inline int tcp_hdr_size (tcp_hdr_t * hdr) {
    return hdr->shdr * 4;
}

#if DBG_DISP_ENABLED(DBG_TCP)       // 注意头文件要包含dbg.h和net_cfg.h
void tcp_show_info (char * msg, tcp_t * tcp);
void tcp_display_pkt (char * msg, tcp_hdr_t * tcp_hdr, pktbuf_t * buf);
void tcp_show_list (void);
#else
#define tcp_show_info(msg, tcp)
#define tcp_display_pkt(msg, hdr, buf)
#define tcp_show_list()
#endif

net_err_t tcp_init(void);

sock_t* tcp_create (int family, int protocol);

net_err_t tcp_close(struct _sock_t* sock);
net_err_t tcp_connect(struct _sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);

#endif // TCP_H

