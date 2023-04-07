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
#include "ipv4.h"


/**
 * @brief 检查包是否有错误
 */
static net_err_t is_pkt_ok(icmpv4_pkt_t * pkt, int size, pktbuf_t * buf) {
    // 比头部空间小，有问题
    if (size <= sizeof(icmpv4_hdr_t)) {
        dbg_warning(DBG_ICMP, "size error: %d", size);
        return NET_ERR_SIZE;
    }

    // 校验和检查
    uint16_t checksum = pktbuf_checksum16(buf, size, 0, 1);
    if (checksum != 0) {
        dbg_warning(DBG_ICMP, "Bad checksum %0x(correct is: %0x)\n", pkt->hdr.checksum, checksum);
        return NET_ERR_CHKSUM;
    }

    return NET_ERR_OK;
}

/**
 *  @brief ICMP输入报文处理
 * ICMP本身没有数据长字段，需要通过ip头来确定，所以这里加size参数，由ip层设置
 * 输入进来的包是IP包，因为有可能其它模块会需要处理这个包，需要知道IP包头
 */
net_err_t icmpv4_in(ipaddr_t *src, ipaddr_t * netif_ip, pktbuf_t *buf) {
    dbg_info(DBG_ICMP, "icmp in !\n");

    // 调整头部空间，给出icmp的头. 下面这里预留了ip包，方便以后处理
    ipv4_pkt_t* ip_pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    int iphdr_size = ip_pkt->hdr.shdr * 4;   // ip包头和icmp包头连续
    net_err_t err = pktbuf_set_cont(buf, sizeof(icmpv4_hdr_t) + iphdr_size);
    if (err < 0) {
        dbg_error(DBG_ICMP, "set icmp cont failed");
        return err;
    }
    ip_pkt = (ipv4_pkt_t*)pktbuf_data(buf);      // 重新调整位置

    // 移除IP包头部, ICMP处理不需要
    err = pktbuf_remove_header(buf, iphdr_size);
    if (err < 0) {
        dbg_error(DBG_IP, "remove ip header failed. err = %d\n", err);
        return NET_ERR_SIZE;
    }
    pktbuf_reset_acc(buf);

    // 做一些检查
    icmpv4_pkt_t * icmp_pkt = (icmpv4_pkt_t*)pktbuf_data(buf);
    if ((err = is_pkt_ok(icmp_pkt, buf->total_size, buf)) != NET_ERR_OK) {
        dbg_warning(DBG_ICMP, "icmp pkt error.drop it. err=%d", err);
        return err;
    }
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
