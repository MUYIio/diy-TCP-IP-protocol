/**
 * @brief TCP/IP协议栈初始化
 *
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#include "net.h"
#include "net_plat.h"
#include "exmsg.h"
#include "pktbuf.h"
#include "dbg.h"
#include "netif.h"
#include "loop.h"
#include "ether.h"
#include "tools.h"
#include "timer.h"
#include "arp.h"
#include "ipv4.h"
#include "icmpv4.h"
#include "sock.h"
#include "raw.h"
#include "udp.h"
#include "dns.h"
#include "tcp.h"

/**
 * 协议栈初始化
 */
net_err_t net_init(void) {
    dbg_info(DBG_INIT, "init net...");

    // 各模块初始化
    net_plat_init();

    // 放在前头，提前进行大小端转换，后面初始化的时候可能发数据包
    tools_init();
    exmsg_init();
    pktbuf_init();
    netif_init();
    ether_init();
    net_timer_init();
    arp_init();
    ipv4_init();
    icmpv4_init();
    socket_init();
    raw_init();
    udp_init();
    dns_init();
    tcp_init();

    // 环回接口初始化
    loop_init();
    return NET_ERR_OK;
}

/**
 * 启动协议栈
 */
net_err_t net_start(void) {
    // 启动消息传递机制
    exmsg_start();

    dbg_info(DBG_INIT, "net is running.");
    return NET_ERR_OK;
}