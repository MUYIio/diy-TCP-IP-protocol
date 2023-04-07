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
#include "dbg.h"
#include "tools.h"
#include "protocol.h"

/**
 * @brief 发送状态名称表
 */
const char * tcp_ostate_name (tcp_t * tcp) {
    static const char * state_name[] = {
        [TCP_OSTATE_IDLE] = "idle",
        [TCP_OSTATE_SENDING] = "sending",
        [TCP_OSTATE_REXMIT] = "resending",
        [TCP_OSTATE_PERSIST] = "persist",
    };

    return state_name[tcp->snd.ostate >= TCP_OSTATE_MAX ? TCP_OSTATE_MAX : tcp->snd.ostate];
}


/**
 * @brief 发送TCP包
 */
static net_err_t send_out (tcp_hdr_t * out, pktbuf_t * buf, ipaddr_t * dest, ipaddr_t * src) {
    //tcp_display_pkt("tcp out", out, buf);

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
 * @brief 发送零窗口更新
 */
net_err_t tcp_send_win_update (tcp_t * tcp) {
    // 分配一个TCP包
    pktbuf_t* buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_NONE;
    }

    // 填充数据包
    tcp_hdr_t* out = (tcp_hdr_t*)pktbuf_data(buf);
    out->sport = tcp->base.local_port;
    out->dport = tcp->base.remote_port;
    out->seq = tcp->snd.nxt;
    out->ack = tcp->rcv.nxt;
    out->flags = 0;
    out->f_ack = 1;                             // ACK标志
    out->win = (uint16_t)tcp_rcv_window(tcp);
    out->urgptr = 0;
    tcp_set_hdr_size(out, sizeof(tcp_hdr_t));

    // 由于可能用于建立连接中，此时tcp的远端地址可能还为空，因此使用seg中的
    return send_out(out, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

/**
 * @brief 发送一个不含数据的纯ACK包
 * 该包不需要进行缓存，直接发送出去
 */
net_err_t tcp_send_ack(tcp_t* tcp, tcp_seg_t * seg) {
    // 不需要对RST包发送ACK响应
    if (seg->hdr->f_rst) {
        return NET_ERR_OK;
    }

    // 分配一个TCP包
    pktbuf_t* buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_NONE;
    }

    // 填充数据包
    tcp_hdr_t* out = (tcp_hdr_t*)pktbuf_data(buf);
    out->sport = tcp->base.local_port;
    out->dport = tcp->base.remote_port;
    out->seq = tcp->snd.nxt;
    out->ack = tcp->rcv.nxt;
    out->flags = 0;
    out->f_ack = 1;                             // ACK标志
    out->win = (uint16_t)tcp_rcv_window(tcp);
    out->urgptr = 0;
    tcp_set_hdr_size(out, sizeof(tcp_hdr_t));

    // 由于可能用于建立连接中，此时tcp的远端地址可能还为空，因此使用seg中的
    return send_out(out, buf, &seg->remote_ip, &seg->local_ip);
}

/**
 * @brief 发送Keepalive报文
 */
net_err_t tcp_send_keepalive(tcp_t* tcp) {
   // 分配一个TCP包
    pktbuf_t* buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_NONE;
    }

    // 填充数据包
    tcp_hdr_t* out = (tcp_hdr_t*)pktbuf_data(buf);
    out->sport = tcp->base.local_port;
    out->dport = tcp->base.remote_port;
    out->seq = tcp->snd.nxt - 1;        // 要求序号为对方期望的序号-1
    out->ack = tcp->rcv.nxt;
    out->flags = 0;
    out->f_ack = 1;
    out->win = (uint16_t)tcp_rcv_window(tcp);
    out->urgptr = 0;
    tcp_set_hdr_size(out, sizeof(tcp_hdr_t));

    return send_out(out, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

/**
 * @brief 发送Keepalive报文
 */
net_err_t tcp_send_reset_for_tcp(tcp_t* tcp) {
   // 分配一个TCP包
    pktbuf_t* buf = pktbuf_alloc(sizeof(tcp_hdr_t));
    if (!buf) {
        dbg_error(DBG_TCP, "no buffer");
        return NET_ERR_NONE;
    }

    // 填充数据包
    tcp_hdr_t* out = (tcp_hdr_t*)pktbuf_data(buf);
    out->sport = tcp->base.local_port;
    out->dport = tcp->base.remote_port;
    out->seq = tcp->snd.nxt;        // 要求序号为对方期望的序号-1
    out->ack = tcp->rcv.nxt;
    out->flags = 0;
    out->f_ack = 1;
    out->f_rst = 1;
    out->win = (uint16_t)tcp_rcv_window(tcp);
    out->urgptr = 0;
    tcp_set_hdr_size(out, sizeof(tcp_hdr_t));

    return send_out(out, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

/**
 * @brief 往TCP包中写入选项数据
 * 要求写之前已经为选项分配了相应的空间并且数据是连续的
 */
static void write_sync_option (tcp_t * tcp, pktbuf_t * buf) {
    int opt_len = sizeof(tcp_opt_mss_t) + sizeof(tcp_opt_sack_t);
    opt_len = (opt_len + 3) / 4 * 4;        // 向上对齐到4字节

    // 这里实际上不太可能出错，TCP包的大小通常是能容纳这些数据的，因为只含包头
    // 所以要不要加点判断呢？
    net_err_t err = pktbuf_resize(buf, buf->total_size + opt_len);
    dbg_assert(err >= 0, "resize error, pktbuf block size too small");

    int len = 0;

    // MSS选项, 可能包含其它选项，以后再加
    tcp_opt_mss_t* opt = (tcp_opt_mss_t *)(((tcp_pkt_t *)pktbuf_data(buf))->data);
    opt->kind = TCP_OPT_MSS;
    opt->length = sizeof(tcp_opt_mss_t);
    opt->mss = x_ntohs(tcp->mss);
    len += sizeof(tcp_opt_mss_t);

    // 通知对方自己支持SACk
    tcp_opt_sack_t * sack = (tcp_opt_sack_t *)((uint8_t *)opt + sizeof(tcp_opt_mss_t));
    sack->kind = TCP_OPT_SACK;
    sack->length = 2;
    len += sizeof(tcp_opt_sack_t);

    // 填充到4字节对齐
    uint8_t * start = (uint8_t *)sack + sizeof(tcp_opt_sack_t);
    while (len++ < opt_len) {
        *start++ = TCP_OPT_NOP;
    }
}

/**
 * @brief 将发送缓存中的数据拷贝到数据包链表中
 */
static int copy_send_data (tcp_t * tcp, pktbuf_t * buf, int doff, int dlen) {
    if (dlen == 0) {
        return 0;
    }

    // 增加缓冲中的数据量
    net_err_t err = pktbuf_resize(buf, (int)(buf->total_size + dlen));
    if (err < 0) {
        dbg_error(DBG_TCP, "pktbuf resize error");
        return -1;
    }
    
    // 定位到数据区域，将数据拷贝至缓存中，下面不应该出错
    int hdr_size = tcp_hdr_size((tcp_hdr_t *)pktbuf_data(buf));
    pktbuf_reset_acc(buf);
    pktbuf_seek(buf, hdr_size);
    tcp_buf_read_send(&tcp->snd.buf, doff, buf, dlen);
    return dlen;
}

/**
 * @brief 计算待发的数据量长（不含FIN和SYN）
 * 可能出现缓存中有数据，但是可发的数据量为0的情况
 */
static void get_send_info (tcp_t * tcp, int rexmit, int no_newdata, int * doff, int * dlen) {
    if (rexmit) {
        // 重发时，从una开始发送，在已有的数据缓存中偏移量为0
        *doff = 0;
        if (no_newdata) {
            *dlen = tcp->snd.nxt - tcp->snd.una;    // 仅未确认的数据
        } else {
            // 获取所有未发送的数据
            *dlen = tcp_buf_cnt(&tcp->snd.buf) - *doff;
            if (*dlen == 0) {
                return;
            }        
        }
    } else {
        // 非重发时，从nxt开始发送，在已有的数据缓存中偏移量为nxt开始的字节偏移
        // 但是要注意，在发送SYN报文时，由于未得到了对方的seq序号，所以不能使用nxt-una
        // 且此时由于SYN肯定是第一个报文，所以发送偏移必须是0
        *doff = tcp->flags.syn_out ? 0 : tcp->snd.nxt - tcp->snd.una;

        // 获取所有未发送的数据
        *dlen = tcp_buf_cnt(&tcp->snd.buf) - *doff;
        if (*dlen == 0) {
            return;
        }
    }

    if (tcp->snd.win == 0) {
        // 窗口大小为0时，只发送1字节数据用于发送探查报文
        *dlen = 1;
    } else {
        // 非零窗口, 首先不能超过MSS
        *dlen = (*dlen > tcp->mss) ? tcp->mss : *dlen; 

        // 如果整体数据量（未发+未确认）超过窗口大小，则缩减整体大小
        // 只发送整个窗口区域内的未发送灵气
        if (*dlen + *doff > tcp->snd.win) {
            *dlen = tcp->snd.win - *doff;
        } 
    } 
}

/**
 * @brief 检查是否有数据需要发送。如果有数据需要发且能发送，则发送
 * 从下面的实现代码可以出现，发送过程中可能会按照需要组成不确定大小的数据包发送
 */
net_err_t tcp_transmit(tcp_t * tcp) {
    // 注意要考虑FIN和SYN，且不能把需要重发的算进去，只算此次新的发的数据
    int dlen, doff;
    get_send_info(tcp, 0, 0, &doff, &dlen);
    if (dlen < 0) {
        return NET_ERR_OK;
    }

    // 由于发送可能由应用发起，也可能是在收到ACK后发起。因此，需要检查当前是否可以发送
    // 如果没有需要可以发送的，并且当前状态不允许发送，则直接退出
    int seq_len = dlen;
    if (tcp->flags.syn_out) {
        seq_len++;
    } 
    
    // FIN仅在缓存为空时才发送
    if ((tcp_buf_cnt(&tcp->snd.buf) == 0) && tcp->flags.fin_out) {
        seq_len++;
    }

    if (seq_len == 0) {
        return NET_ERR_OK;
    }

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
    if (hdr->f_syn) {
        // syn置位，写入SYN选项, 首次连接，告诉对方自己的一些信息
        write_sync_option(tcp, buf);
    }

    // 整个TCP传输中，除第一次传递之外，其它都需要发送ACK
    hdr->f_ack = tcp->flags.irs_valid;
    hdr->win = (uint16_t)tcp_rcv_window(tcp);
    hdr->urgptr = 0;                             // 暂不支持紧急数据
    tcp_set_hdr_size(hdr, buf->total_size);

    // 拷贝要发送的数据，具体写的数据长度，由MSS、接收窗口、缓冲区中数据三者来决定
    // 即实际发送的可能要小一些
    copy_send_data(tcp, buf, doff, dlen);

    // 当FIN标志位置位，且此次发送的数据为整个缓冲中所有的数据时，FIN才需要发送出去
    // 否则，应当等所有的数据都被发送完毕时，FIN才应当被发送
    if (tcp->flags.fin_out) {
        hdr->f_fin = (tcp_buf_cnt(&tcp->snd.buf) == 0) ? 1 : 0;
    }

    // 调整序号, 加上有效数据量、SYN和FIN标志位
    tcp->snd.nxt += dlen + hdr->f_syn + hdr->f_fin;

    // 发送出去
    dbg_info(DBG_TCP, "tcp send: dlen %d, seqlen: %d, %s", dlen, seq_len, tcp_ostate_name(tcp));
    return send_out(hdr, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

/**
 * @brief 从TCP发送缓存中提取数据后重发
 * 数据重发时，如果no_data置1，则从una开始，提取最大能发送的数据量进行发送
 */
net_err_t tcp_retransmit(tcp_t* tcp, int no_newdata) {
    // 注意要考虑FIN和SYN，且不能把需要重发的算进去，只算此次新的发的数据
    int dlen, doff;
    get_send_info(tcp, 1, no_newdata, &doff, &dlen);
    if (dlen < 0) {
        return NET_ERR_OK;
    }
    // 由于发送可能由应用发起，也可能是在收到ACK后发起。因此，需要检查当前是否可以发送
    // 如果没有需要可以发送的，并且当前状态不允许发送，则直接退出
    int seq_len = dlen;
    if (tcp->flags.syn_out) {
        seq_len++;
    } 
    
    if (tcp->flags.fin_out) {
        seq_len++;
    }

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
    hdr->seq = tcp->snd.una;        // 从una开始发送
    hdr->ack = tcp->rcv.nxt;
    hdr->flags = 0;  
    hdr->f_syn = tcp->flags.syn_out;     // 发送后不清理，因为可能要重传
   if (hdr->f_syn) {
        // syn置位，写入SYN选项, 首次连接，告诉对方自己的一些信息
        write_sync_option(tcp, buf);
    }

    // 整个TCP传输中，除第一次传递之外，其它都需要发送ACK
    hdr->f_ack = tcp->flags.irs_valid;
    hdr->win = (uint16_t)tcp_rcv_window(tcp);
    hdr->urgptr = 0;                        // 不支持紧急数据
    tcp_set_hdr_size(hdr, buf->total_size);

    // 拷贝要发送的数据，从偏移量为0开始发送
    copy_send_data(tcp, buf, doff, dlen);

    // 当FIN标志位置位，且此次发送的数据为整个缓冲中所有的数据时，FIN才需要发送出去
    // 否则，应当等所有的数据都被发送完毕时，FIN才应当被发送
    if (tcp->flags.fin_out) {
        hdr->f_fin = (tcp_buf_cnt(&tcp->snd.buf) == 0) ? 1 : 0;
    }

    // 计算此次重发，有多少新数据被发送，将其统计到snd.nxt中
    // 不必考虑SYN，重发时SYN肯定是之前已经发过了，不能计算在内
    // 也不必考虑FIN，因为之前的transmit，FIN肯定也是已经发过不了，所以不计算在内
    int diff = tcp->snd.una + dlen - tcp->snd.nxt;
    tcp->snd.nxt += diff > 0 ? diff: 0;

    // 发送出去
    dbg_info(DBG_TCP, "tcp send: seq %u, ack %u, dlen %d, seqlen: %d, %s", 
                hdr->seq, hdr->ack, dlen, seq_len, tcp_ostate_name(tcp));
    return send_out(hdr, buf, &tcp->base.remote_ip, &tcp->base.local_ip);
}

/**
 * @brief 发送SYN报文
 * 先缓存标志，该标志会在能够发送时再发送出去
 */
net_err_t tcp_send_syn(tcp_t* tcp) {
    tcp->flags.syn_out = 1;

    // 无延迟的立即发送
    tcp_out_event(tcp, TCP_OEVENT_SEND);
    return NET_ERR_OK;
}

/**
 * @brief 发送FIN报文
 * 先缓存标志，该标志会在能够发送时再发送出去
 */
net_err_t tcp_send_fin (tcp_t* tcp) {
    tcp->flags.fin_out = 1;

    // 无延迟的立即发送
    tcp_out_event(tcp, TCP_OEVENT_SEND);
    return NET_ERR_OK;
}

/**
 * @brief 开始RTO计算
 */
static void tcp_begin_rto (tcp_t * tcp) {
    if (tcp->flags.rto_going == 0) {
        tcp->snd.rttseq = tcp->snd.nxt;
        sys_time_curr(&tcp->snd.rtttime);
        dbg_warning(DBG_TCP, "rttseq %u", tcp->snd.rttseq);
        dbg_warning(DBG_TCP, "rtttime %u", tcp->snd.rtttime);
        tcp->flags.rto_going = 1;
    }
}

/**
 * @brief 更新TCP重传超时使用的RTO
 * 参考TCPIP详解和https://zhuanlan.zhihu.com/p/541727333
 * TCP/IP详解中给出的公式
 *      Err = RTT - srtt;
 *      srtt = srtt + g(Err), g = 1/8
 *      rttvar = rttvar + h(|Err| - rttvar), h=1/4
 *      RTO = srtt + 4*(rttvar)
 * 进行变换，以免移位丢失数据
 *      Err = RTT - srtt
 *      8*srtt = 8*srtt + Err
 *      4*rttvar = 4*rttvar + |Err| - rttvar
 *      RTO = srtt + 4*rttvar
 * 在对发送方和接收方之间发送的 segment 进行往返时间（RTT）测量之前，发送方应设置 RTO < -1秒 
 * 当第一次测量到 RTT 的值 R 时，主机必须进行以下初始化：
 *      SRTT=R
 *      RTTVAR = R/2 
 *      RTO = SRTT + max {G , K ∗ RTTVAR}
 *      其中 K = 4 
 */
void tcp_cal_rto (tcp_t * tcp) {
    // 仅在RTO计算时才计算
    if (tcp->flags.rto_going == 0) {
        return;
    }

    // 从定时器中获得RTT
    net_timer_t * stimer = &tcp->snd.timer;
    int rtt = sys_time_goes(&tcp->snd.rtttime);
    if (rtt == 0) {
        rtt = 1;            // ???
    }

    if (tcp->snd.srtt != 0) {
        // 非第一次计算
        int delta = rtt - (tcp->snd.srtt >> 3);     // Err = RTT - srtt
        tcp->snd.srtt += delta;      // 8*srtt = 8*srtt + Err

        // 4*rttvar = 4*rttvar + |Err| - rttvar
        tcp->snd.rttvar += ((delta < 0 ? -delta : delta)) - (tcp->snd.rttvar >> 2);  
    } else {
        // 第一次计算RTO, 根据该计算结果：RTO的值应为3个RTT
        tcp->snd.srtt = rtt << 3;           // 8*srtt <- M           
        tcp->snd.rttvar = rtt << 1;         // 4*rttvar <- M/2
        tcp->snd.rttseq = tcp->snd.nxt;     // 记录下当前的序号
    }

    // RTO = srtt + 4*rttvar
    tcp->snd.rto = (tcp->snd.srtt >> 3)+ tcp->snd.rttvar;
    if (tcp->snd.rto < TCP_RTO_MIN) {
        tcp->snd.rto = TCP_RTO_MIN;
    }
    tcp->flags.rto_going = 0;

    // 显示调试信息
    dbg_warning(DBG_TCP, "rttseq: %u, rto: %d, rtt: %d, srtt: %d, rttvar: %d",
                    tcp->snd.rttseq, tcp->snd.rto, rtt, tcp->snd.srtt >> 3,  tcp->snd.rttvar >> 2);
    dbg_warning(DBG_TCP, "rtttime %u", tcp->snd.rtttime);
}    

/**
 * @brief 结束RTO计算
 */
static inline void tcp_end_rto (tcp_t * tcp) {
    tcp->flags.rto_going = 0;
}

/**
 * @brief 根据输入的报文，调整发送窗口的大小
 */
void tcp_cal_snd_win (tcp_t * tcp, tcp_seg_t * seg) {
    tcp_hdr_t * tcp_hdr = (tcp_hdr_t *)seg->hdr;

    // 序号要比上一次更新的要大，不能是更老的包，因为老的包可能窗口比现在的大。这会是一个错误
    if (TCP_SEQ_LT(tcp->snd.wl1_seq, seg->seq) ||
        // 或者相同，但是ACK至少要和上次的相同。
        // 为什么要相同？因为那个窗口探查回复的报文，seq和ack是保持不变的，所以要加上相同
        ((tcp->snd.wl1_seq == seg->seq) && (TCP_SEQ_LE(tcp->snd.wl2_ack, tcp_hdr->ack)))) {
        // 纪录一下发送窗口大小
        int pre_win = tcp->snd.win;
        tcp->snd.win = tcp_hdr->win;
        tcp->snd.wl1_seq = seg->seq;
        tcp->snd.wl2_ack = tcp_hdr->ack;

        // 如果窗口变小，不处理。后续处理数据时会自动处理窗口变0的问题
        if (tcp->snd.win <= pre_win) {
            return;
        }
    }
}

/*
 * @brief 处理TCP报文中的ACK
 * 收到了ACK，即有部分数据得到了确认，在这里进行处理
 */
net_err_t tcp_ack_process (tcp_t * tcp, tcp_seg_t * seg) {
    tcp_hdr_t * tcp_hdr = seg->hdr;

    // 要求必须: una < ack <= nxt，这个包才是这次可以被接收的
    if (TCP_SEQ_LT(tcp_hdr->ack, tcp->snd.una)) {
        // 重复ACK，进行快速重传。这里处理的比较简单
        if ((tcp->snd.ostate == TCP_OSTATE_SENDING) && (++tcp->snd.dup_ack >= TCP_DUPTHRESH)) {
            tcp_retransmit(tcp, 0);
            tcp_set_ostate(tcp, TCP_OSTATE_REXMIT);
            tcp->snd.dup_ack = 0;
        } else {
            tcp_cal_snd_win(tcp, seg);
        }
        // 仍然可能数据，可继续进行处理
        return NET_ERR_OK;
    } else if (tcp_hdr->ack == tcp->snd.una) {
        // 相同，不需要确认任何数据，注意更新窗口。因为可能收到窗口更新包
        tcp_cal_snd_win(tcp, seg);
        return NET_ERR_OK;
    } else if (TCP_SEQ_LT(tcp->snd.nxt, tcp_hdr->ack)) {
        // 如果 > nxt，确认了没有发的区域，不正确，要丢掉
        if (tcp->state == TCP_STATE_SYN_RECVD) {
            // SYN_RCVD状态，之前已经收到了对方的连接请求，发送RST复位
            tcp_send_reset(seg);
        } else {
            // 其它状态，发送ACK, 再次通知对方自己的正确序号和ACK
            tcp_send_ack(tcp, seg);
        }

        // 此ACK不可被接受
        return NET_ERR_UNREACH;
    } 

    tcp->snd.dup_ack = 0;
    
    // 调整窗口
    tcp_cal_snd_win(tcp, seg);

    // 先消耗掉SYN标志位
    if (tcp->flags.syn_out) {
        tcp->snd.una++;
        tcp->flags.syn_out = 0;
    }

    // 如果数据区有空间，则移除数据区中已经确认的数量，并唤醒等待写的任务 
    int acked_cnt = tcp_hdr->ack - tcp->snd.una;        // 此次确认的序号长度
    int unacked_cnt = tcp->snd.nxt - tcp->snd.una;      // TCP中未确认的序号长度
    int curr_acked = (acked_cnt > unacked_cnt) ? unacked_cnt : acked_cnt;  // 选其中较小的部分

    // 移除缓存中的数据量
    tcp->snd.una += curr_acked;
    curr_acked -= tcp_buf_remove(&tcp->snd.buf, curr_acked);   // 可能包含FIN
    if (tcp_buf_cnt(&tcp->snd.buf) == 0) {
        // 并且FIN被确认时，清除OUT_FIN，进入IDLE状态，即完成所有数据的发送
        if (curr_acked && (tcp->flags.fin_out)) {
            tcp->flags.fin_out = 0;
        } 
    }

    // 唤醒任务
    sock_wakeup(&tcp->base, SOCK_WAIT_WRITE, NET_ERR_OK);

    // 最后计算RTT和RTO
    // 之前的设置：tcp->snd.rttseq = tcp->snd.nxt;
    if (TCP_SEQ_LE(tcp->snd.rttseq, tcp->snd.una)) {
        // 最后的RTT测试包已经被接收，计算RTO
        tcp_cal_rto(tcp);
    }
    return NET_ERR_OK;
}

/**
 * @brief 将数据写入发送缓存中，必须的时候启动发送过程
 * 注意有可能不能一次性全部写入数据 
 */
int tcp_write_sndbuf(tcp_t * tcp, const uint8_t * buf, int len) {
    // 检查发送缓存
    int free_cnt = tcp_buf_free_cnt(&tcp->snd.buf);
    if (free_cnt <= 0) {
        // 缓存已满立即退出
        return 0;
    }

    // 计算实际能写入的大小
    int wr_len = (len > free_cnt) ? free_cnt : len;
    tcp_buf_write_send(&tcp->snd.buf, buf, wr_len);
    return wr_len;
}

/**
 * @brief TCP发送定时处理
 * 对发送超时、重传超时、坚持定时等进行超时处理
 */
static void tcp_out_timer_tmo (struct _net_timer_t* timer, void * arg) {
    tcp_t * tcp = (tcp_t *)arg;

    // 作为警告，方便观察
    dbg_warning(DBG_TCP, "timer tmo: %s", tcp_ostate_name(tcp));
    
    // 根据状态做不同的处理
    switch (tcp->snd.ostate) {
        case TCP_OSTATE_SENDING: {
            // 超时，结束RTO的计算
            tcp_end_rto(tcp);

            // 发送状态超时，那么此时就应该进入重传状态, 重发所有数据
            net_err_t err = tcp_retransmit(tcp, 1);
            if (err < 0) {
                dbg_error(DBG_TCP, "rexmit failed.");
                return;
            }

            // 进入重发状态, 启动重传定时器。然后重传有几次渐进变化的过程
            tcp->snd.rexmit_cnt = 1;
            tcp->snd.rto <<= 1;
            tcp->snd.ostate = TCP_OSTATE_REXMIT;
            net_timer_add(&tcp->snd.timer, tcp_ostate_name(tcp), tcp_out_timer_tmo, tcp, tcp->snd.rto, 0);
            break;
        }
        case TCP_OSTATE_REXMIT: {
            // 什么时候取消重传，应当是数据发完之后，迟迟未收到响应

            if ((++tcp->snd.rexmit_cnt > TCP_RESENDING_RETRIES)) {
                // 重传超时，大概率也会接收超时。由于应用层可能在超时后退出close
                // 而close也会导致发送数据包，进一步超时，所以这就处理有些问题
                // 因此，最简单的方式就是直接中止整个传输
                dbg_error(DBG_TCP, "rexmit tmo err");
                tcp_abort(tcp, NET_ERR_TMO);
                return;    
            }

            // 继续重发
            net_err_t err = tcp_retransmit(tcp, 1);
            if (err < 0) {
                dbg_error(DBG_TCP, "rexmit failed.");
                return;
            }

            // 重传超时，karn算法第二部分：使用二进制指数退避处理重启定时器
            // 但是这个时长不太长，太长了会导致这个等待时间过长，甚至超过1年
            tcp->snd.rto <<= 1;
            if (tcp->snd.rto >= TCP_RTO_MAX) {
                tcp->snd.rto = TCP_RTO_MAX;
            }
            net_timer_add(&tcp->snd.timer, tcp_ostate_name(tcp), tcp_out_timer_tmo, tcp, tcp->snd.rto, 0);
            break;
       }
        case TCP_OSTATE_PERSIST: {
            if (TCP_PERSIST_RETRIES && (++tcp->snd.rexmit_cnt > TCP_PERSIST_RETRIES)) {
                // 通知产生了坚持定时器超时
                dbg_error(DBG_TCP, "persist tmo err");
                sock_wakeup(&tcp->base, SOCK_WAIT_WRITE, NET_ERR_TMO);
                return;    
            }

            // 继续发送探查报文，内部只有1个字节
            net_err_t err = tcp_retransmit(tcp, 1);
            if (err < 0) {
                dbg_error(DBG_TCP, "send win query failed.");
                return;
            }

            // 重启定时器，在一个RTO之后， 同样采用指数退避的算法。这里不更改RTO
            int tmo = tcp->snd.rto << tcp->snd.rexmit_cnt;
            if (tmo >= TCP_RTO_MAX) {
                tmo = TCP_RTO_MAX;
            }
            net_timer_add(&tcp->snd.timer, tcp_ostate_name(tcp), tcp_out_timer_tmo, tcp, tmo, 0);
            break;
        }
        default:
            dbg_error(DBG_TCP, "tcp state error: %d", tcp->state);
            return;
    }
}

/**
 * @brief 更改输出状态
 */
void tcp_set_ostate (tcp_t * tcp, tcp_ostate_t state) {
    // 状态检查
    if (state >= TCP_OSTATE_MAX) {
        dbg_error(DBG_TCP, "unknown state: %d", tcp->snd.ostate);
        return;
    }

    // 允许重复设置，这样以便于重启定时器，特别是对于连续发送多个包的情况
    // 当发送很长的数据时，时间较长，如果只是在第一次发包时记事，那么长时间发送后
    //if (tcp->snd.ostate == state) {
    //    return;
    //}

    // 根据状态的不同，做不同的定时处理
    int tmo = 0;
    switch (state) {
    case TCP_OSTATE_IDLE:
        tcp->snd.ostate = state;
        net_timer_remove(&tcp->snd.timer);     // 移除定时器
        return;
    case TCP_OSTATE_SENDING:
        tmo = tcp->snd.rto;
        break;
    case TCP_OSTATE_REXMIT:
        tmo = tcp->snd.rto;     // 仍然使用RTO
        break;
    case TCP_OSTATE_PERSIST:
        tmo = TCP_PERSIST_TMO;
        tcp->snd.rexmit_cnt = 0;
        break;
    default:
        break;
    }

    // 更换状态，启动定时器
    tcp->snd.ostate = state;
    net_timer_remove(&tcp->snd.timer);     // 移除定时器
    net_timer_add(&tcp->snd.timer, tcp_ostate_name(tcp), tcp_out_timer_tmo, tcp, tmo, 0);
    //dbg_info(DBG_TCP, "tcp ostate:%s", tcp_ostate_name(tcp));
}  

/**
 * @brief 尽可能性一次性的发送大量的包，不含重传的数据
 */
static void tcp_transmit_most (tcp_t * tcp) {
    // 尽可能按窗口的大小将所有的数据发完
    int total = tcp->snd.win;         // 发送窗口大小
    int data_count = tcp_buf_cnt(&tcp->snd.buf);        // 我们自己的数据
    if (total > data_count) {     
        // 取二者最小值，即我们当前发送的数据
        total = data_count;
    }
    // 再减掉未确认的，余下就是能能发的数据部分
    total -= tcp->snd.nxt - tcp->snd.una;
    total += tcp->flags.syn_out;     // 别忘了这两个
    if ((tcp_buf_cnt(&tcp->snd.buf) == 0) && tcp->flags.fin_out) {
        total++;
    }

    while (total) {
        int nxt = tcp->snd.nxt;
        tcp_transmit(tcp);
        total -= tcp->snd.nxt - nxt;
     }
}

/**
 * @brief 空闲状态下的事件处理
 */
static void tcp_ostate_idle_in (tcp_t * tcp, tcp_oevent_t event) {
    switch (event) {
    case TCP_OEVENT_SEND:
        // 如果窗口不为0，，即可以发送，那么发送数据进入发送状态
        if (tcp->snd.win) {
            // 发送数据可能多的数据
            tcp_transmit_most(tcp);
            tcp_set_ostate(tcp, TCP_OSTATE_SENDING);

            // 正常发送，开始计算RTT
            tcp_begin_rto(tcp);
        } else {
            // 想发，但是窗口为0，则等待窗口为0的恢复
            // 也有可能存在窗口为0且只发FIN的情况，这种很少见，不处理，等窗口恢复再发
            tcp_transmit(tcp);
            tcp_set_ostate(tcp, TCP_OSTATE_PERSIST);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief 发送状态处理
 */
static void tcp_ostate_sending_in (tcp_t * tcp, tcp_oevent_t event) {
    switch (event) {
    case TCP_OEVENT_SEND: 
        // 应用端在数据未发时时，继续请求发送
        if (tcp->snd.win) {
            // 发送数据可能多的数据
            tcp_transmit_most(tcp);

            // 重启定时器，这样发送定时器统计的是最后一个包超时和RTT
            tcp_set_ostate(tcp, TCP_OSTATE_SENDING);

            // 正常发送，开始计算RTT
            tcp_begin_rto(tcp);
        } else {
            // 想发，但是窗口为0，则等待窗口为0的恢复
            // 也有可能存在窗口为0且只发FIN的情况，这种很少见，不处理，等窗口恢复再发
            tcp_transmit(tcp);
            tcp_set_ostate(tcp, TCP_OSTATE_PERSIST);
        }
        break;
    case TCP_OEVENT_XMIT:
        // 所有发送的数据已经被确，v即最后一个包被确认
        // 如果只是被确认了一部分，则说明还没有发送完毕，让超时自动发生并重传
        // 而如果是全部确认了，要么继续发送，要么进入空闲状态
        // 注意FIN，FIN在所有数据发完之后再发，所以有可能una==nxt但FIN还未发出去
        if ((tcp->snd.una == tcp->snd.nxt) || tcp->flags.fin_out) {
            // 如果还有数据要发送，继续发送；
           if (tcp_buf_cnt(&tcp->snd.buf) || tcp->flags.fin_out) {
                // 发送前需要检查窗口，如果为0则进入坚持状态，否则会进入普通发送状态
                if (tcp->snd.win) {
                    tcp_transmit_most(tcp);
                    tcp_set_ostate(tcp, TCP_OSTATE_SENDING);

                    // 正常发送，开始计算RTT
                    tcp_begin_rto(tcp);
                } else {
                    tcp_set_ostate(tcp, TCP_OSTATE_PERSIST);
                    tcp_transmit(tcp);
                }
            } else {
                // 没有数据要发，进入空闲状态
                tcp_set_ostate(tcp, TCP_OSTATE_IDLE); 
            }
        }
        break;
    default:
        break;
    }
}

/**
 * @brief 重传处理
 */
static void tcp_ostate_rexmit_in (tcp_t * tcp, tcp_oevent_t event) {
    switch (event) {
        case TCP_OEVENT_XMIT: {
            // 所有发送的数据已经被确认，如果还有数据要发送，继续发送；否则进入空闲态
            // 注意FIN，FIN在所有数据发完之后再发，所以有可能una==nxt但FIN还未发出去
            if ((tcp->snd.una == tcp->snd.nxt) || tcp->flags.fin_out) {
                if (tcp_buf_cnt(&tcp->snd.buf) || tcp->flags.fin_out) {
                    // 发送前需要检查窗口，如果为0则进入坚持状态，否则会进入普通发送状态
                    if (tcp->snd.win) {
                        tcp_transmit_most(tcp);
                        tcp_set_ostate(tcp, TCP_OSTATE_SENDING);
                   } else {
                        tcp_set_ostate(tcp, TCP_OSTATE_PERSIST);
                        tcp_transmit(tcp);
                    }
                } else {
                    tcp_set_ostate(tcp, TCP_OSTATE_IDLE);
                }
            } else {
                // 如果只确认了一部分该如何处理？只重发未确认的，还是加上未发送的？
                if (tcp->snd.win) {
                    dbg_info(DBG_TCP, "rxmit ack in, retransmit, seq: %u, ack: %u", tcp->snd.una, tcp->rcv.nxt);
                    tcp_set_ostate(tcp, TCP_OSTATE_REXMIT);
                } else {
                    tcp_set_ostate(tcp, TCP_OSTATE_PERSIST);
                }
                tcp_retransmit(tcp, 0);
            }
            break;
        }
        default:
            break;
    }
}

/**
 * @brief 坚持状态下的事件处理 
 * 这个状态是否要计算RTO呢？不计算吧，因为只是短暂的
 */
static void tcp_ostate_persist_in (tcp_t * tcp, tcp_oevent_t event) {
    switch (event) {
    case TCP_OEVENT_XMIT: {
        // 数据数据被确认完毕，有数据需要发送，则进入发送状态，没有进入idle状态
        // 如果没有数据需要发送，则进入重传状态
        if (tcp->snd.win) {
            if ((tcp->snd.una == tcp->snd.nxt) || tcp->flags.fin_out) {
                // 数据数据全部确认，即下次需要发新的，则进入普通发送状态
                tcp_transmit_most(tcp);
                tcp_set_ostate(tcp, TCP_OSTATE_SENDING);
            } else { 
                // 有数据未确认，进入重传状态，重新传输数据
                tcp_set_ostate(tcp, TCP_OSTATE_REXMIT);
                tcp_retransmit(tcp, 1);
            }        
        }
        break;
    }
    default:
        break;
    }
}

/**
 * @brief 向外输出数据
 */
void tcp_out_event (tcp_t * tcp, tcp_oevent_t event) {
    static void (*state_fun[]) (tcp_t * tcp, tcp_oevent_t event) = {
        [TCP_OSTATE_IDLE] = tcp_ostate_idle_in,
        [TCP_OSTATE_SENDING] = tcp_ostate_sending_in,
        [TCP_OSTATE_REXMIT] = tcp_ostate_rexmit_in,
        [TCP_OSTATE_PERSIST] = tcp_ostate_persist_in,
    };

    if (tcp->snd.ostate >= TCP_OSTATE_MAX) {
        dbg_error(DBG_TCP, "tcp ostate unknown: %d", tcp->snd.ostate);
        return;
    }

    // 执行回调函数
    state_fun[tcp->snd.ostate](tcp, event);
}