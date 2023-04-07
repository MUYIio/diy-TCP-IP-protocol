/**
 * @file tcp_out.c
 * @author lishutong
 * @brief TCP发送处理模块，包含基本的数据报、SYN报文、FIN报文发送以及重发处理
 * @version 0.1
 * @date 2022-10-22
 * 
 * @copyright Copyright (c) 2022
 * 在发送数据时，该模块会自动根据实际的配置从发送缓冲中读取数据进行组包。
 * 包的大小是不固定的，目的是减少分片以及最大可能发送多的数据。
 * 
 * 发送时，不会为每个报文段设置不同的重传定时，而是发送一个窗口的数据，只设置一个
 * 定时器，当ACK到达时再更新超时。如果未及时到达，则重发报文段。
 */
#include "tcp.h"
#include "tcp_out.h"
#include "tcp_in.h"
#include "dbg.h"
#include "tools.h"
#include "protocol.h"

/**
 * @brief 发送TCP包
 */
static net_err_t send_out (tcp_hdr_t * out, pktbuf_t * buf, ipaddr_t * dest, ipaddr_t * src) {
    tcp_display_pkt("tcp out", out, buf);

    // 大小端转换处理
    out->sport = x_htons(out->sport);
    out->dport = x_htons(out->dport);
    out->seq = x_htonl(out->seq);
    out->ack = x_htonl(out->ack);
    out->win = x_htons(out->win);
    out->urgptr = x_htons(out->urgptr);

    // 校验和计算等
    out->checksum = 0;
    out->checksum = checksum_peso(dest->a_addr, src->a_addr, NET_PROTOCOL_TCP, buf);
    
    // 通过IP层发送出去

    net_err_t err = ipv4_out(NET_PROTOCOL_TCP, dest, src, buf);
    if (err < 0) {
        dbg_info(DBG_TCP, "send tcp buf error");
        pktbuf_free(buf);
    }
    
    return err;
}

/**
 * @brief 给输入的报文发送一个rst回复。如果输入报文是rst报文，则忽略
 * 复位数据包需要立即发送，不需要进行重发操作
 */
net_err_t tcp_send_reset(tcp_seg_t * seg) {
    // 不需要对复位报文发送复位报文, 否则对方也如此的话，将会一直发送RST包
    tcp_hdr_t * in = seg->hdr;
    if (in->f_rst) {
        dbg_info(DBG_TCP, "reset, ignore");
        return NET_ERR_OK;
    }

    // 分配一个TCP包，RST包没有数据，只要包头即可
    pktbuf_t* buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_warning(DBG_TCP, "no pktbuf");
        return NET_ERR_NONE;
    }

    // 生成TCP包头
    tcp_hdr_t* out = (tcp_hdr_t*)pktbuf_data(buf);
    out->sport = in->dport;
    out->dport = in->sport;
    out->flags = 0;
    out->f_rst = 1;

    // SEQ: 通过SEQ告诉对方已经收到了这个包，并且正对这个包响应。正确设置保证这个包可被对方接收
    // ACK: 告诉对方之前的某个序号之前的数据已经收到，请发后面的过来。
    if (in->f_ack) {
        // 收到对方ACK，则使用ACK号作为起始序号，这样才能在对方接收的范围区间
        out->seq = in->ack;

        // 由于设置上面的序号就能保证被对方正确接收，所以不用再配置ACK。下面可以不填
        out->ack = 0;
        out->f_ack = 0;        // 复位和RESET标志
    } else {
        // 未收对方ACK（比如只有SYN的报文），即对方没有说明期望收到的序号，起始序号不知道填多少，所以填0
        out->seq = 0;

        // 不知道对方想要收到的数据序号是多少，填了序号0
        // 但是又要让对方能够接收，对方并不是随便发个RST包都会接受的
        // 此时，对方检查的是ACK包（见SYN_SENT状态中的处理），检查发现ACK在合适的区间时才会处理
        // 即对方会检查这个是否对上次发的包的响应， 因此，这里就需要发送ACK给对方了。
        out->ack = in->seq + seg->seq_len;
        out->f_ack = 1;        // 复位和RESET标志
     }
    tcp_set_hdr_size(out, sizeof(tcp_hdr_t));

    out->win = out->urgptr = 0;         // 窗口和紧急指针不需要使用
    return send_out(out, buf, &seg->remote_ip, &seg->local_ip);
}


/**
 * @brief 检查是否有数据需要发送。如果有数据需要发且能发送，则发送
 * 从下面的实现代码可以出现，发送过程中可能会按照需要组成不确定大小的数据包发送
 */
net_err_t tcp_transmit(tcp_t * tcp) {
    // 分配一个TCP包，暂不考虑选项区域和头部区域
    pktbuf_t* buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_OK;
    }

    // 生成数据包头
    tcp_hdr_t* hdr = (tcp_hdr_t*)pktbuf_data(buf);
    hdr->sport = tcp->base.local_port;
    hdr->dport = tcp->base.remote_port;
    hdr->seq = tcp->snd.nxt;
    hdr->ack = tcp->rcv.nxt;
    hdr->flags = 0;
    hdr->f_syn = tcp->flags.syn_out;   // 复位完后不清理，因为可能要重传

    // 整个TCP传输中，除第一次传递之外，其它都需要发送ACK
    hdr->f_ack = 0;
    hdr->win = 1024;        // 暂时填这个
    hdr->urgptr = 0;                             // 暂不支持紧急数据
    tcp_set_hdr_size(hdr, buf->total_size);


    // 调整序号, 加上有效数据量、SYN和FIN标志位
    tcp->snd.nxt += hdr->f_syn + hdr->f_fin;

    // 发送出去
    return send_out(hdr, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

/**
 * @brief 发送SYN报文
 * 先缓存标志，该标志会在能够发送时再发送出去
 */
net_err_t tcp_send_syn(tcp_t* tcp) {
    tcp->flags.syn_out = 1;

    // 无延迟的立即发送
    tcp_transmit(tcp);
    return NET_ERR_OK;
}
