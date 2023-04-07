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
#include "protocol.h"
#include "tools.h"

/**
 * @brief 显示icmp包
 */
#if DBG_DISP_ENABLED(DBG_ICMP)
static void display_icmp_packet(char * title, icmpv4_pkt_t  * pkt) {
    plat_printf("--------------- %s ------------------ \n", title);
    plat_printf("type: %d\n", pkt->hdr.type);
    plat_printf("code: %d\n", pkt->hdr.code);
    plat_printf("checksum: %x\n", x_ntohs(pkt->hdr.checksum));
    plat_printf("------------------------------------- \n");
}
#else
#define display_icmp_packet(title, packet)
#endif //debug_icmp

/**
 * @brief 发送icmp包
 */
static net_err_t icmpv4_out(ipaddr_t* dest, ipaddr_t* src, pktbuf_t* buf) {
    icmpv4_pkt_t* pkt = (icmpv4_pkt_t*)pktbuf_data(buf);

    // 重新定位到icmp包头
    pktbuf_seek(buf, 0);
    pkt->hdr.checksum = pktbuf_checksum16(buf, buf->total_size, 0, 1);

    // 然后再直接发回去
    display_icmp_packet("icmp reply", pkt);
    return ipv4_out(NET_PROTOCOL_ICMPv4, dest, src, buf);
}

/**
 * @brief 发送icmp echo响应
 */
static net_err_t icmpv4_echo_reply(ipaddr_t *dest, ipaddr_t * src, pktbuf_t *buf) {
    icmpv4_pkt_t* pkt = (icmpv4_pkt_t*)pktbuf_data(buf);

    // 这里不用调整连续性，因为是直接修改了输入包，输入包是连续的

    // 只修改类型和校验和，其余数据完全保持不变
    pkt->hdr.type = ICMPv4_ECHO_REPLY;
    pkt->hdr.checksum = 0;
    return icmpv4_out(dest, src, buf);
}

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
    display_icmp_packet("icmp in", icmp_pkt);

    // 根据类型做不同的处理
    switch (icmp_pkt->hdr.type) {
        case ICMPv4_ECHO_REQUEST: {
            dbg_dump_ip(DBG_ICMP, "icmp request, ip:", src);
            return icmpv4_echo_reply(src, netif_ip, buf);
        }
        default: {
            pktbuf_free(buf);
            dbg_dump_ip(DBG_ICMP, "unknown type icmp packet, ip:", src);
            return NET_ERR_OK;
        }
    }
}

/**
 * 初始化icmp模块
 */
net_err_t icmpv4_init(void) {
    dbg_info(DBG_ICMP, "init icmp");
    dbg_info(DBG_ICMP, "done");
    return NET_ERR_OK;
}
