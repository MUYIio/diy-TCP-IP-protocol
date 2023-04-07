#ifndef TCP_H
#define TCP_H

#include "sock.h"
#include "net_cfg.h"
#include "dbg.h"
#include "pktbuf.h"
#include "tcp_buf.h"

#define TCP_DEFAULT_MSS     536

#define TCP_OPT_END             0
#define TCP_OPT_NOP             1
#define TCP_OPT_MSS             2

#pragma pack(1)

typedef struct _tcp_opt_mss_t {
    uint8_t kind;
    uint8_t length;
    uint16_t mss;
}tcp_opt_mss_t;

typedef struct _tcp_hdr_t {
    uint16_t sport;
    uint16_t dport;
    uint32_t seq;
    uint32_t ack;
    union 
    {
        uint16_t flag;
    
#if NET_ENDIAN_LITTLE
        struct {
            uint16_t resv : 4;
            uint16_t shdr : 4;
            uint16_t f_fin : 1;
            uint16_t f_syn : 1;
            uint16_t f_rst : 1;
            uint16_t f_psh : 1;
            uint16_t f_ack : 1;
            uint16_t f_urg : 1;
            uint16_t f_ece : 1;
            uint16_t f_cwr : 1;
        };
#else
        struct {
            uint16_t shdr : 4;
            uint16_t resv : 4;
            uint16_t f_cwr : 1;
            uint16_t f_ece : 1;
            uint16_t f_urg : 1;
            uint16_t f_ack : 1;
            uint16_t f_psh : 1;
            uint16_t f_rst : 1;
            uint16_t f_syn : 1;
            uint16_t f_fin : 1;
        };
#endif
    };
    uint16_t win;
    uint16_t checksum;
    uint16_t urgptr;
}tcp_hdr_t;


typedef struct _tcp_pkt_t {
    tcp_hdr_t hdr;
    uint8_t data[1];
}tcp_pkt_t;

#pragma pack()

typedef struct _tcp_seg_t {
    ipaddr_t local_ip;
    ipaddr_t remote_ip;
    tcp_hdr_t * hdr;
    pktbuf_t * buf;
    uint32_t data_len;
    uint32_t seq;
    uint32_t seq_len;
}tcp_seg_t;

typedef enum _tcp_state_t {
    TCP_STATE_FREE = 0,
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

typedef struct _tcp_t {
    sock_t base;

    struct {
        uint32_t syn_out : 1;       // 1 - syn已经发送
        uint32_t fin_out : 1;
        uint32_t irs_valid : 1;     
    }flags;

    tcp_state_t state;
    int mss;

    struct {
        sock_wait_t wait;
    }conn;

    struct {
        tcp_buf_t buf;
        uint8_t data[TCP_SBUF_SIZE];

        
        uint32_t una;
        uint32_t nxt;
        uint32_t iss;
        sock_wait_t wait;
    }snd;

    struct {
        uint32_t nxt;
        uint32_t iss;
        sock_wait_t wait;
    }rcv;
}tcp_t;

#if DBG_DISP_ENABLED(DBG_TCP)
void tcp_show_info (char * msg, tcp_t * tcp);
void tcp_show_pkt (char * msg, tcp_hdr_t * tcp_hdr, pktbuf_t * buf);
void tcp_show_list (void);
#else
#define tcp_show_info(msg, tcp)
#define tcp_show_pkt(msg, hdr, buf)
#define tcp_show_list()
#endif

net_err_t tcp_init (void);
sock_t * tcp_create (int family, int protocol);
net_err_t tcp_abort (tcp_t * tcp, net_err_t err);

void tcp_read_options(tcp_t * tcp, tcp_hdr_t * tcp_hdr);

tcp_t * tcp_find(ipaddr_t * local_ip, uint16_t local_port, ipaddr_t * remote_ip, uint16_t remote_port);

static inline int tcp_hdr_size (tcp_hdr_t * hdr) {
    return hdr->shdr * 4;
}

static inline void tcp_set_hdr_size(tcp_hdr_t * hdr, int size) {
    hdr->shdr = size / 4;
}

#endif
