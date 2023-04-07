/**
 * @file tcp_in.h
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
#ifndef TCP_IN_H
#define TCP_IN_H

#include "tcp.h"

net_err_t tcp_in(pktbuf_t *buf, ipaddr_t *src_ip, ipaddr_t *dest_ip);
void tcp_seg_init (tcp_seg_t * seg, pktbuf_t * buf, ipaddr_t * local, ipaddr_t * remote);
net_err_t tcp_data_in (tcp_t * tcp, tcp_seg_t * seg);

#endif // TCP_IN_H
