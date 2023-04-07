/**
 * @file udp.c
 * @author lishutong(527676163@qq.com)
 * @brief UDP协议层处理，以及sock接口的实现
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 */
#include <string.h>
#include "dbg.h"
#include "udp.h"
#include "ipaddr.h"
#include "mblock.h"
#include "socket.h"

static udp_t udp_tbl[UDP_MAX_NR];           // UDP控制块数组
static mblock_t udp_mblock;                 // 空闲UDP控制块链表
static nlist_t udp_list;                     // 已绑定的控制块链表


/**
 * 检查端口是被使用
 */
static int is_port_used(int port) {
    nlist_node_t * node;

    nlist_for_each(node, &udp_list) {
        sock_t* sock = nlist_entry(node, sock_t, node);
        if (sock->local_port == port) {
            return 1;
        }
    }

    return 0;
}

/**
 * 为sock分配本地端口号
 */
static net_err_t alloc_port(sock_t* sock) {
    static int search_index = NET_PORT_DYN_START;

    // 遍历所有动态端口
    for (int i = NET_PORT_DYN_START; i < NET_PORT_DYN_END; i++) {
        int port = search_index++;

        // 没有使用则返回
        if (!is_port_used(port)) {
            sock->local_port = port;
            return NET_ERR_OK;
        }
    }

    // 找不到有效的端口号
    return NET_ERR_NONE;
}

/**
 * @brief 通过UDP向指定地址发送数据
 * 如果已经连接，再用sendto，则直接使用传过来的地址，不检查地址是否一致
 * 可以看到，UDP是每次应用写入多少数据，就做成一个UDP包往下发送
 */
net_err_t udp_sendto (struct _sock_t * sock, const void* buf, size_t len, int flags, const struct x_sockaddr* dest,
            x_socklen_t dest_len, ssize_t * result_len) {
    struct x_sockaddr_in* addr = (struct x_sockaddr_in*)dest;

    // 如果已经连接，则检查二者是否一致，不一致则报错
    // 与raw不同，这里还要检查端口号
    ipaddr_t dest_ip;
    ipaddr_from_buf(&dest_ip, addr->sin_addr.addr_array);
    uint16_t dport = x_ntohs(addr->sin_port);
    if (!ipaddr_is_any(&sock->remote_ip)) {
        if (!ipaddr_is_equal(&dest_ip, &sock->remote_ip) || (dport != sock->remote_port)) {
            dbg_error(DBG_RAW, "udp is connected");
            return NET_ERR_CONNECTED;
        }
    }

    // 本地可能没有绑定端口，此时需要分配一个
    if (!sock->local_port && ((sock->err = alloc_port(sock)) < 0)) {
        dbg_error(DBG_UDP, "no port avaliable");
        return NET_ERR_NONE;
    }

    // 分配缓存空间, 预留目的地址
    pktbuf_t* pktbuf = pktbuf_alloc((int)len);
    if (!pktbuf) {
        dbg_error(DBG_UDP, "no buffer");
        return NET_ERR_MEM;
    }

    // 数据拷贝过去
    net_err_t err = pktbuf_write(pktbuf, (uint8_t*)buf, (int)len);
    if (err < 0) {
        dbg_error(DBG_UDP, "copy data error");
        goto end_sendto;
    }

    // 通过UDP层发送出去
    err = udp_out(&dest_ip, dport, &sock->local_ip, sock->local_port, pktbuf);
    if (err < 0) {
        dbg_error(DBG_UDP, "send error");
        goto end_sendto;
    }

    if (result_len) {
        *result_len = (ssize_t)len;
    }
    return NET_ERR_OK;
end_sendto:
    pktbuf_free(pktbuf);
    return err;
}

/**
 * @brief 创建一个socket套接字
 */
sock_t* udp_create(int family, int protocol) {
    // UDP特有的sock操作列表
    static const sock_ops_t udp_ops = {
        .sendto = udp_sendto,
        .setopt = sock_setopt,
    };

    // 分配一个raw sock结构
    udp_t* udp = (udp_t *)mblock_alloc(&udp_mblock, 0);
    if (!udp) {
        dbg_error(DBG_UDP, "no sock");
        return (sock_t*)0;
    }

    // 初始化通用的sock结构部分
    net_err_t err = sock_init((sock_t*)udp, family, protocol, &udp_ops);
    if (err < 0) {
        dbg_error(DBG_UDP, "create failed.");
        mblock_free(&udp_mblock, udp);
        return (sock_t*)0;
    }
    nlist_init(&udp->recv_list);

    // 接收时需要等待，其它不需要等
    udp->base.rcv_wait = &udp->rcv_wait;
    if (sock_wait_init(udp->base.rcv_wait) < 0) {
        dbg_error(DBG_UDP, "create rcv.wait failed");
        goto create_failed;
    }

    // 插入全局队列中, 注意此时端口号和IP地址全为0，所以实际不匹配任何连接
    nlist_insert_last(&udp_list, &udp->base.node);
    return (sock_t*)udp;
create_failed:
    sock_uninit((sock_t *)udp);
    return (sock_t *)0;
}

/**
 * @brief UDP模块初始化
 */
net_err_t udp_init(void) {
    dbg_info(DBG_UDP, "udp init.");

    // 应用端可能有多任务访问，因此需要锁
    mblock_init(&udp_mblock, udp_tbl, sizeof(udp_t), UDP_MAX_NR, NLOCKER_NONE);
    nlist_init(&udp_list);

    dbg_info(DBG_UDP, "init done.");
    return NET_ERR_OK;
}

/**
 * @brief 将数据包发送给指定的IP和端口
 */
net_err_t udp_out(ipaddr_t * dest, uint16_t dport, ipaddr_t * src, uint16_t sport, pktbuf_t * buf) {
    dbg_info(DBG_UDP, "send an udp packet!");

    // src地址为空时，查找路由表，从而获得出口的IP地址
    if (!src || ipaddr_is_any(src)) {
        rentry_t* rt = rt_find(dest);
        if (rt == (rentry_t*)0) {
            dbg_dump_ip(DBG_UDP, "no route to dest: ", dest);
            return NET_ERR_UNREACH;
        }

        // 发送IP为网卡的IP地址
        src = &rt->netif->ipaddr;
    }

    // 发送前，添加UDP头部
    net_err_t err = pktbuf_add_header(buf, sizeof(udp_hdr_t), 1);
    if (err < 0) {
        dbg_error(DBG_UDP, "add header failed. err = %d", err);
        return NET_ERR_SIZE;
    }

    // 填充包头，端口号等可从udp中获取
    udp_hdr_t * udp_hdr = (udp_hdr_t*)pktbuf_data(buf);
    udp_hdr->src_port = sport;
    udp_hdr->dest_port = dport;
    udp_hdr->total_len = buf->total_size;
    udp_hdr->checksum = 0;

    // 数据层转换并计算伪校验和
    udp_hdr->src_port = x_htons(udp_hdr->src_port);
    udp_hdr->dest_port = x_htons(udp_hdr->dest_port);
    udp_hdr->total_len = x_htons(udp_hdr->total_len);
    udp_hdr->checksum = 0; // checksum_peso(src->a_addr, dest->a_addr, NET_PROTOCOL_UDP, buf);;

    // 交由IP层发送, tos和ttl暂定为固定值
    err = ipv4_out(NET_PROTOCOL_UDP, dest, src, buf);
    if (err < 0) {
        dbg_error(DBG_UDP, "udp out error, err = %d", err);
        return err;
    }

    return err;
}
