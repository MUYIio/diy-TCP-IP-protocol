/**
 * @file tcp_in.c
 * @author lishutong
 * @brief TCP输入处理, 包括对其中的数据进行管理
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022
 * 
 * 如果收到了TCP包，可能需要即地其进行排序，因为TCP可能会先收到
 * 大序列号的数据，再收到小的；甚至有可能出现多个包之间序列号重叠。
 * 即处理数据空洞、乱序、重复等多种问题。
 */
#include "socket.h"
#include "tcp_in.h"

/**
 * @brief TCP数据报的输入处理
 * 注意:传入进来的包为IP数据包
 */
net_err_t tcp_in(pktbuf_t *buf, ipaddr_t *src_ip, ipaddr_t *dest_ip) {    
    // 先查验TCP包头的校验和
    tcp_hdr_t * tcp_hdr = (tcp_hdr_t *)pktbuf_data(buf);
    if (tcp_hdr->checksum) {
        pktbuf_reset_acc(buf);
        if (checksum_peso(dest_ip->a_addr, src_ip->a_addr, NET_PROTOCOL_TCP, buf)) {
            dbg_warning(DBG_TCP, "tcp checksum incorrect");
            return NET_ERR_CHKSUM;
        }
    }

    // 检查包的合法性，只做初步的检查，不检查序号等内容
    // 大小应当至少和包头一样大小
    if ((buf->total_size < sizeof(tcp_hdr_t)) || (buf->total_size < tcp_hdr_size(tcp_hdr))) {
        dbg_warning(DBG_TCP, "tcp packet size incorrect: %d!", buf->total_size);
        return NET_ERR_SIZE;
    }

    // 端口不能为空
    if (!tcp_hdr->sport || !tcp_hdr->dport) {
        dbg_warning(DBG_TCP, "port == 0");
        return NET_ERR_UNREACH;
    }

    // 标志位不能为空，总有一个置位
    if (tcp_hdr->flags == 0) {
        dbg_warning(DBG_TCP, "flag == 0");
        return NET_ERR_UNREACH;
    }

    // 调整大小端
    tcp_hdr->sport = x_ntohs(tcp_hdr->sport);
    tcp_hdr->dport = x_ntohs(tcp_hdr->dport);
    tcp_hdr->seq = x_ntohl(tcp_hdr->seq);
    tcp_hdr->ack = x_ntohl(tcp_hdr->ack);
    tcp_hdr->win = x_ntohs(tcp_hdr->win);
    tcp_hdr->urgptr = x_ntohs(tcp_hdr->urgptr);

    // 后续处理
    return NET_ERR_OK;
}

