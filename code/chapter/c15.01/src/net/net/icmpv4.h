/**
 * @file icmpv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>
#include "netif.h"
#include "pktbuf.h"

net_err_t icmpv4_init(void);
net_err_t icmpv4_in(ipaddr_t *src_ip, ipaddr_t* netif_ip, pktbuf_t *buf);

#endif // ICMP_H
