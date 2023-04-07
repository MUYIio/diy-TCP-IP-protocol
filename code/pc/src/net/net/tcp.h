#ifndef TCP_H
#define TCP_H

#include "sock.h"
#include "timer.h"
#include "exmsg.h"
#include "dbg.h"
#include "net_cfg.h"
#include "tcp_buf.h"

#pragma pack(1)

#define TCP_DEFAULT_MSS     536         // 缺省的MSS值

// TCP选项数据
#define TCP_OPT_END        0            // 选项结束
#define TCP_OPT_NOP        1            // 无操作，用于填充
#define TCP_OPT_MSS        2            // 最大段大小
#define TCP_OPT_WSOPT      3            // 窗口缩放因子 
#define TCP_OPT_SACK       4            // SACK选项

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
 * @brief TCP选项
 */
typedef struct _tcp_opt_mss_t {
    uint8_t kind;
    uint8_t length;
    union {
        uint16_t mss;
    };
}tcp_opt_mss_t;

/**
 * @brief SACK选项
 */
typedef struct _tcp_opt_sack_t {
    uint8_t kind;
    uint8_t length;
}tcp_opt_sack_t;

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
 * @brief TCP输出状态
 */
typedef enum _tcp_ostate_t {
    TCP_OSTATE_IDLE,                // 空闲状态，没有数据要发送
    TCP_OSTATE_SENDING,             // 正在发送报文
    TCP_OSTATE_REXMIT,              // 重发状态，正在重传数据
    TCP_OSTATE_PERSIST,             // 窗口为0，正在发送探测报文

    TCP_OSTATE_MAX,
}tcp_ostate_t;

/**
 * TCP连接块
 */
typedef struct _tcp_t {
    sock_t base;            // 基础sock结构
    struct _tcp_t * parent; // 父tcp结构
    struct {
        // 不要写成int，因为会被解释成—1?
        uint32_t syn_out : 1;        // 输出SYN标志位
        uint32_t fin_in : 1;         // 收到FIN
        uint32_t fin_out : 1;        // 需要发送FIN
        uint32_t irs_valid : 1;      // 初始序列号IRS不可用
        uint32_t rto_going : 1;      // 正在计算RTO
        uint32_t keep_enable : 1;     // 是否开启keepalive
        uint32_t inactive : 1;       // 非激活状态
    }flags;

    tcp_state_t state;      // TCP状态
    int mss;                // mss值

    struct {
        sock_wait_t wait;       // 连接等待结构
        int keep_idle;          // Keepalive空闲时间
        int keep_intvl;         // 超时间隔
        int keep_cnt;           // 最大检查次数
        int keep_retry;         // 当前重试次数
        int backlog;            // 接受队列的大小
        net_timer_t keep_timer; // 超时定时器
    }conn;

    struct {
        tcp_buf_t buf;      // 发送缓冲区
        uint8_t  data[TCP_SBUF_SIZE];
        uint32_t una;	    // 已发但未确认区域的起始序号
        uint32_t nxt;	    // 未发送的起始序号
        uint32_t iss;	    // 起始发送序号
        int win;            // 对方接收窗口大小
        tcp_ostate_t ostate;  // 输出状态
        net_timer_t timer;    // 发送定时器
        uint32_t wl1_seq;   // 最后一次用来更新窗口的分片序号
        uint32_t wl2_ack;   // 最后一次用来更新窗口的分片ACK号

        int rexmit_cnt;     // 重发次数
        int rexmit_max;     // 最大重发次数
        net_time_t rtttime; // 计算RTT时的时间
        uint32_t rttseq;    // 计算RTT时的序列号
        int srtt;           // smoothed round-trip time
        int rttvar;         // round-trip time variation
        int rto;            // 重传超时
        int dup_ack;        // 快速重传ACK次数
        int persist_retry;  // 零窗口重试次数
        
        sock_wait_t wait;   // 发送等待结构
    }snd;

    struct {
        tcp_buf_t buf;      // 接收缓冲区
        uint8_t data[TCP_RBUF_SIZE];
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

sock_t* tcp_find(ipaddr_t * local_ip, uint16_t local_port, ipaddr_t * remote_ip, uint16_t remote_port);
void tcp_insert (tcp_t * tcp);
int tcp_conn_exist(sock_t * sock);
int tcp_alloc_port(void);

void tcp_free(tcp_t* tcp);

void tcp_read_options(tcp_t *tcp, tcp_hdr_t *tcp_hdr);
int tcp_rcv_window (tcp_t * tcp);

net_err_t tcp_abort (tcp_t * tcp, int err);

int tcp_backlog_count (tcp_t * tcp);
void tcp_set_local(sock_t* sock, ipaddr_t* ipaddr, int port);

sock_t* tcp_create (int family, int protocol);
void tcp_kill_all_timers (tcp_t * tcp);

net_err_t tcp_close(struct _sock_t* sock);
net_err_t tcp_connect(struct _sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);
void tcp_destory (struct _sock_t * sock);
net_err_t tcp_recv (struct _sock_t* s, void* buf, size_t len, int flags, ssize_t * result_len);
net_err_t tcp_send (struct _sock_t* sock, const void* buf, size_t len, int flags, ssize_t * result_len);
net_err_t tcp_bind(struct _sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len);
net_err_t tcp_listen (struct _sock_t* s, int backlog);
net_err_t tcp_accept (struct _sock_t *s, struct x_sockaddr* addr, x_socklen_t* len, struct _sock_t ** client);
tcp_t * tcp_create_child (tcp_t * parent, tcp_seg_t * seg);

void tcp_keepalive_start (tcp_t * tcp, int run);
void tcp_keepalive_restart (tcp_t * tcp);

#define TCP_SEQ_LE(a, b)        ((int32_t)(a) - (int32_t)(b) <= 0)
#define TCP_SEQ_LT(a, b)        ((int32_t)(a) - (int32_t)(b) < 0)

#endif // TCP_H

