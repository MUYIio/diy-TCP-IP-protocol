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
#include <string.h>
#include "icmpv4.h"
#include "dbg.h"

/**
 *  @brief ICMP输入报文处理
 * ICMP本身没有数据长字段，需要通过ip头来确定，所以这里加size参数，由ip层设置
 * 输入进来的包是IP包，因为有可能其它模块会需要处理这个包，需要知道IP包头
 */
net_err_t icmpv4_in(ipaddr_t *src, ipaddr_t * netif_ip, pktbuf_t *buf) {
    return NET_ERR_SIZE;
}

/**
 * 初始化icmp模块
 */
net_err_t icmpv4_init(void) {
    dbg_info(DBG_ICMP, "init icmp");
    dbg_info(DBG_ICMP, "done");
    return NET_ERR_OK;
}
