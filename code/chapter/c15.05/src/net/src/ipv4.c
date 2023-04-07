/**
 * @file ipv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "ipv4.h"
#include "dbg.h"
#include "tools.h"
#include "protocol.h"
#include "icmpv4.h"

static uint16_t packet_id = 0;                  // 每次发包的序号

/**
 * @brief 设置IP包头的首部长度
 */
static inline void set_header_size(ipv4_pkt_t* pkt, int size) {
    pkt->hdr.shdr = size / 4;
}

/**
 * @brief IP模块初始化
 */
net_err_t ipv4_init(void) {
    dbg_info(DBG_IP,"init ip\n");

    dbg_info(DBG_IP,"done.");
    return NET_ERR_OK;
}

/**
 * @brief 对IP包头进行大小端转换
 */
static void iphdr_ntohs(ipv4_pkt_t* pkt) {
    pkt->hdr.total_len = x_ntohs(pkt->hdr.total_len);
    pkt->hdr.id = x_ntohs(pkt->hdr.id);
    pkt->hdr.frag_all = x_ntohs(pkt->hdr.frag_all);
}

/**
 * @brief 将IP包头的格式转换为网络端格式
 */
static void iphdr_htons(ipv4_pkt_t* pkt) {
    pkt->hdr.total_len = x_htons(pkt->hdr.total_len);
    pkt->hdr.id = x_htons(pkt->hdr.id);
    pkt->hdr.frag_all = x_ntohs(pkt->hdr.frag_all);
}

#if DBG_DISP_ENABLED(DBG_IP)

/**
 * @brief 打印IP数据包
 */
static void display_ip_packet(ipv4_pkt_t* pkt) {
    ipv4_hdr_t* ip_hdr = (ipv4_hdr_t*)&pkt->hdr;

    plat_printf("--------------- ip ------------------ \n");
    plat_printf("    Version:%d\n", ip_hdr->version);
    plat_printf("    Header len:%d bytes\n", ipv4_hdr_size(pkt));
    plat_printf("    Totoal len: %d bytes\n", ip_hdr->total_len);
    plat_printf("    Id:%d\n", ip_hdr->id);
    plat_printf("    TTL: %d\n", ip_hdr->ttl);
    plat_printf("    Protocol: %d\n", ip_hdr->protocol);
    plat_printf("    Header checksum: 0x%04x\n", ip_hdr->hdr_checksum);
    dbg_dump_ip_buf(DBG_IP, "    src ip:", ip_hdr->dest_ip);
    plat_printf("\n");
    dbg_dump_ip_buf(DBG_IP, "    dest ip:", ip_hdr->src_ip);
    plat_printf("\n");
    plat_printf("--------------- ip end ------------------ \n");

}

#else
#define display_ip_packet(pkt)
#endif

/**
 * @brief 非IP包分片处理的输入处理
 */
static net_err_t ip_normal_in(netif_t* netif, pktbuf_t* buf, ipaddr_t* src, ipaddr_t * dest) {
    ipv4_pkt_t* pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    display_ip_packet(pkt);

    // 根据不同协议类型做不同处理， 进行多路径分解
    switch (pkt->hdr.protocol) {
    case NET_PROTOCOL_ICMPv4: {
        net_err_t err = icmpv4_in(src, &netif->ipaddr, buf);
        if (err < 0) {
            dbg_warning(DBG_IP, "icmp in failed.\n");
            return err;
        }
        return NET_ERR_OK;
    }
    case NET_PROTOCOL_UDP:
        // 发送ICMP不可达信息
        iphdr_htons(pkt);       // 注意转换回来
        icmpv4_out_unreach(src, &netif->ipaddr, ICMPv4_UNREACH_PORT, buf);
        break;
    case NET_PROTOCOL_TCP:
        break;
    default:
        dbg_warning(DBG_IP, "unknown protocol %d, drop it.\n", pkt->hdr.protocol);
        break;
    }

    // 处理不了，上层释放
    return NET_ERR_UNREACH;
}

/**
 * @brief 发送一个IP数据包
 *
 * 具体发送时，会根据是否超出MTU值，来进行分片发送。
 */
net_err_t ipv4_out(uint8_t protocol, ipaddr_t* dest, ipaddr_t * src, pktbuf_t* buf) {
    dbg_info(DBG_IP,"send an ip packet.\n");

    // 调整读写位置，预留IP包头，注意要连续存储
    net_err_t err = pktbuf_add_header(buf, sizeof(ipv4_hdr_t), 1);
    if (err < 0) {
        dbg_error(DBG_IP, "no enough space for ip header, curr size: %d\n", buf->total_size);
        return NET_ERR_SIZE;
    }

    // 构建IP数据包
    ipv4_pkt_t * pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    pkt->hdr.shdr_all = 0;
    pkt->hdr.version = NET_VERSION_IPV4;
    set_header_size(pkt, sizeof(ipv4_hdr_t));
    pkt->hdr.total_len = buf->total_size;
    pkt->hdr.id = packet_id++;        // 计算不断自增
    pkt->hdr.frag_all = 0;         //
    pkt->hdr.ttl = NET_IP_DEF_TTL;
    pkt->hdr.protocol = protocol;
    pkt->hdr.hdr_checksum = 0;
    ipaddr_to_buf(src, pkt->hdr.src_ip);
    ipaddr_to_buf(dest, pkt->hdr.dest_ip);

    // 大小端转换
    iphdr_htons(pkt);

    // 计算校验和
    pktbuf_reset_acc(buf);
    pkt->hdr.hdr_checksum = pktbuf_checksum16(buf, ipv4_hdr_size(pkt), 0, 1);

    // 开始发送
    display_ip_packet(pkt);
    err = netif_out(netif_get_default(), dest, buf);
    if (err < 0) {
        dbg_warning(DBG_IP, "send ip packet failed. error = %d\n", err);
        return err;
    }

    return NET_ERR_OK;
}

/**
 * @brief 检查包是否有错误
 */
static net_err_t is_pkt_ok(ipv4_pkt_t* pkt, int size) {
    // 版本检查，只支持ipv4
    if (pkt->hdr.version != NET_VERSION_IPV4) {
        dbg_warning(DBG_IP, "invalid ip version, only support ipv4!\n");
        return NET_ERR_NOT_SUPPORT;
    }

    // 头部长度要合适
    int hdr_len = ipv4_hdr_size(pkt);
    if (hdr_len < sizeof(ipv4_hdr_t)) {
        dbg_warning(DBG_IP, "IPv4 header error: %d!", hdr_len);
        return NET_ERR_SIZE;
    }

    // 总长必须大于头部长，且<=缓冲区长
    // 有可能xbuf长>ip包长，因为底层发包时，可能会额外填充一些字节
    int total_size = x_ntohs(pkt->hdr.total_len);
    if ((total_size < sizeof(ipv4_hdr_t)) || (size < total_size)) {
        dbg_warning(DBG_IP, "ip packet size error: %d!\n", total_size);
        return NET_ERR_SIZE;
    }

    // 校验和为0时，即为不需要检查检验和
    if (pkt->hdr.hdr_checksum) {
        uint16_t c = checksum16((uint16_t*)pkt, hdr_len, 0, 1);
        if (c != 0) {
            dbg_warning(DBG_IP, "Bad checksum: %0x(correct is: %0x)\n", pkt->hdr.hdr_checksum, c);
            return 0;
        }
    }

    return NET_ERR_OK;
}

/**
 * @brief IP输入处理, 处理来自netif的buf数据包（IP包，不含其它协议头）
 */
net_err_t ipv4_in(netif_t* netif, pktbuf_t* buf) {
    dbg_info(DBG_IP, "IP in!\n");

    // 调整包头位置，保证连续性，方便接下来的处理
    net_err_t err = pktbuf_set_cont(buf, sizeof(ipv4_hdr_t));
    if (err < 0) {
        dbg_error(DBG_IP, "adjust header failed. err=%d\n", err);
        return err;
    }

    // 预先做一些检查
    ipv4_pkt_t* pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    if (is_pkt_ok(pkt, buf->total_size) != NET_ERR_OK) {
        dbg_warning(DBG_IP, "packet is broken. drop it.\n");
        return err;
    }

    // 预先进行转换，方便后面进行处理，不用再临时转换
    iphdr_ntohs(pkt);

    // buf的整体长度可能比ip包要长，比如比较小的ip包通过以太网发送时
    // 由于底层包的长度有最小的要求，因此可能填充一些0来满足长度要求
    // 因此这里将包长度进行了缩小
    err = pktbuf_resize(buf, pkt->hdr.total_len);
    if (err < 0) {
        dbg_error(DBG_IP, "ip packet resize failed. err=%d\n", err);
        return err;
    }

    // 判断IP数据包是否是给自己的，不是给自己的包不处理
    ipaddr_t dest_ip, src_ip;
    ipaddr_from_buf(&dest_ip, pkt->hdr.dest_ip);
    ipaddr_from_buf(&src_ip, pkt->hdr.src_ip);

    // 最简单的判断：与本机网口相同, 或者广播
    if (!ipaddr_is_match(&dest_ip, &netif->ipaddr, &netif->netmask)) {
        pktbuf_free(buf);
        return NET_ERR_UNREACH;
    }

    err = ip_normal_in(netif, buf, &src_ip, &dest_ip);
    return err;
}
