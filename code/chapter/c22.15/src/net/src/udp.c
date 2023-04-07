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

#if DBG_DISP_ENABLED(DBG_UDP)
static void display_udp_list (void) {
    plat_printf("--- udp list\n --- ");

    int idx = 0;
    nlist_node_t * node;

    nlist_for_each(node, &udp_list) {
        udp_t * udp = (udp_t *)nlist_entry(node, sock_t, node);
        plat_printf("[%d]\n", idx++);
        dump_ip_buf("\tlocal:", (const uint8_t *)&udp->base.local_ip.a_addr);
        plat_printf("\tlocal port: %d\n", udp->base.local_port);
        dump_ip_buf("\tremote:", (const uint8_t *)&udp->base.remote_ip.a_addr);
        plat_printf("\tremote port: %d\n", udp->base.remote_port);
    }
}
static void display_udp_packet(udp_pkt_t * pkt) {
    plat_printf("UDP packet:\n");
    plat_printf("source Port:%d\n", pkt->hdr.src_port);
    plat_printf("dest Port: %d\n", pkt->hdr.dest_port);
    plat_printf("length: %d bytes\n", pkt->hdr.total_len);
    plat_printf("checksum:  %04x\n", pkt->hdr.checksum);
}
#else

#define display_udp_packet(packet)
#define display_udp_list()
#endif

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
 * 根据目的IP和端口号，找到对应的sock块
 */
static sock_t* udp_find(ipaddr_t* src_ip, uint16_t sport, ipaddr_t* dest_ip, uint16_t dport) {
    // 遍历整个列表, 找到对应的sock块
    // 可能存在的一种情况是端口不为空，但是IP为空；同时存在另一个sock，端口和IP都不为空
    // 按照下面的算法是先找到哪个用哪个，其实最好两者同时存在时，优化匹配后者(完全匹配), 待优化
    nlist_node_t* node;
    sock_t * found = (sock_t *)0;

    nlist_for_each(node, &udp_list) {
        sock_t* s = nlist_entry(node, sock_t, node);

        // 目的端口必须一致，否则跳出
        if (!dport || (s->local_port != dport)) {
            continue;
        }

        // 检查目的ID，如果指定了，不一样则跳过。如果未指定，则是允许的
        if (!ipaddr_is_any(&s->local_ip) && !ipaddr_is_equal(&s->local_ip, dest_ip)) {
            continue;
        }

        // 检查源ID，如果指定了，不一样则跳过。如果未指定，则是允许的
        if (!ipaddr_is_any(&s->remote_ip) && !ipaddr_is_equal(&s->remote_ip, src_ip)) {
            continue;
        }

        // 检查源端口，如果指定了但不一样则跳过。如果未指定，则允许
        if (s->remote_port && (s->remote_port != sport)) {
            continue;
        }

        // 条件匹配：目的端口 + 本地IP为空或者目的IP=本地IP + 远端IP为空或相同 + 远端端口为0或者相同
        found = s;
        break;
    }
    return found;
}

/**
 * @brief 绑定本地端口。
 * 主要用于服务器，服务器会监听本地IP和某个端口，以便处理收到的数据包
 * 客户端也可用，用于指定本地连接所用的ip和端口号
 *    同一个Socket只可以将1个端口绑定到1个地址上。 （ok）
 *    即使不同的Socket也不能重复绑定相同的地址和端口。  (ok)
 *    不同的Socket可以将不同的端口绑定到相同的IP地址上。 (ok)
 *    不同的Socket可以将相同的端口绑定到不同的IP地址上。 (ok)
 */
net_err_t udp_bind(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len) {
    const struct x_sockaddr_in* addr_in = (const struct x_sockaddr_in*)addr;

    // 不允许重复绑定端口号
    if (sock->local_port != NET_PORT_EMPTY) {
        dbg_error(DBG_UDP, "already binded.");
        return NET_ERR_BINED;
    }

    ipaddr_t local_ip;
    ipaddr_from_buf(&local_ip, (const uint8_t *)&addr_in->sin_addr.addr_array);
    int port = x_ntohs(addr_in->sin_port);

    // 遍历列表，检查是否有IP地址和端口号完全相同的项
    nlist_node_t* node;
    udp_t* udp = (udp_t*)0;
    nlist_for_each(node, &udp_list) {
        udp_t* u = (udp_t *)nlist_entry(node, sock_t, node);
        if ((sock_t*)u == sock) {
            continue;
        }

        if (ipaddr_is_equal(&sock->local_ip, &local_ip) && (sock->local_port == port)) {
            udp = u;
            break;
        }
    }

    // 已经绑定，报错，没有则设置地址进去
    if (udp) {
        dbg_error(DBG_UDP, "port already used!");
        return NET_ERR_BINED;
    } else {
        // 调用基础bind，设置好addr和端口号
        sock_bind(sock, addr, len);
    }

    display_udp_list();
    return NET_ERR_OK;
}

/**
 * @breif 连接到远程主机
 */
net_err_t udp_connect(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len) {
    sock_connect(sock, addr, len);
    display_udp_list();
    return NET_ERR_OK;
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
 * @brief 读取一些数据
 */
net_err_t udp_recvfrom(sock_t* sock, void* buf, size_t len, int flags,
        struct x_sockaddr* src, x_socklen_t* addr_len, ssize_t * result_len) {
    udp_t * udp = (udp_t *)sock;

    // 取出一个数据包
    nlist_node_t * first = nlist_remove_first(&udp->recv_list);
    if (!first) {
        *result_len = 0;
        // 后续任务需要继续等待
        return NET_ERR_NEED_WAIT;
    }

    pktbuf_t* pktbuf = nlist_entry(first, pktbuf_t, node);
    dbg_assert(pktbuf != (pktbuf_t *)0, "pktbuf error");

    // 填充IP地址与端口号地址来源信息
    udp_from_t* from = (udp_from_t *)pktbuf_data(pktbuf);
    struct x_sockaddr_in* addr = (struct x_sockaddr_in*)src;
    plat_memset(addr, 0, sizeof(struct x_sockaddr));
    addr->sin_family = AF_INET;
    addr->sin_port = x_htons(from->port);     // 上层要大端
    ipaddr_to_buf(&from->from, addr->sin_addr.addr_array);

    // 重新定位
    pktbuf_remove_header(pktbuf, sizeof(udp_from_t));       // TODO: 这是原来写了两个？

    // 从底层收到的是IP数据包，包含包头，方便读取各种信息
    int size = (pktbuf->total_size > (int)len) ? (int)len : pktbuf->total_size;
    pktbuf_reset_acc(pktbuf);
    net_err_t err = pktbuf_read(pktbuf, buf, size);
    if (err < 0) {
        pktbuf_free(pktbuf);
        dbg_error(DBG_UDP, "pktbuf read error");
        return err;
    }

    pktbuf_free(pktbuf);

    if (result_len) {
        *result_len = (ssize_t)size;
    }
    return NET_ERR_OK;
}

/**
 * @brief 关闭udp连接
 * 关闭比较简单，直接移除，不需要再通信
 */
net_err_t udp_close(sock_t* sock) {
    udp_t * udp = (udp_t *)sock;

    // 先从队列中移除
    nlist_remove(&udp_list, &sock->node);

    // 释放所有数据包
	nlist_node_t* node;
	while ((node = nlist_remove_first(&udp->recv_list))) {
		pktbuf_t* buf = nlist_entry(node, pktbuf_t, node);
		pktbuf_free(buf);
	}
    mblock_free(&udp_mblock, sock);

    display_udp_list();
    return NET_ERR_OK;
}

/**
 * @brief 创建一个socket套接字
 */
sock_t* udp_create(int family, int protocol) {
    // UDP特有的sock操作列表
    static const sock_ops_t udp_ops = {
        .sendto = udp_sendto,
        .recvfrom = udp_recvfrom,
        .connect = udp_connect,
        .bind = udp_bind,
        .close = udp_close,
        .setopt = sock_setopt,
        .send = sock_send,      // 使用基础sock默认提供的
        .recv = sock_recv,
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
 * @brief 检查包是否有错误
 */
static net_err_t is_pkt_ok(udp_pkt_t * pkt, int size) {
    if ((size < sizeof(udp_hdr_t)) || (size < pkt->hdr.total_len)) {
        dbg_error(DBG_UDP, "udp packet size incorrect: %d!", size);
        return NET_ERR_SIZE;
    }

    return NET_ERR_OK;
}
/**
 * @brief UDP包输入处理
 */
net_err_t udp_in (pktbuf_t* buf, ipaddr_t* src_ip, ipaddr_t* dest_ip) {
    dbg_info(DBG_UDP, "Recv a udp packet!");

    int iphdr_size = ipv4_hdr_size((ipv4_pkt_t *)pktbuf_data(buf));

    // 设置包的连续性，含IP包头和UDP包头
    net_err_t err = pktbuf_set_cont(buf, sizeof(udp_hdr_t) + iphdr_size);
    if (err < 0) {
        dbg_error(DBG_ICMP, "set udp cont failed");
        return err;
    }

    // 定位到UDP包
    ipv4_pkt_t * ip_pkt = (ipv4_pkt_t *)pktbuf_data(buf);   //
    udp_pkt_t* udp_pkt = (udp_pkt_t*)((uint8_t *)ip_pkt + iphdr_size);
    uint16_t local_port = x_ntohs(udp_pkt->hdr.dest_port);
    uint16_t remote_port = x_ntohs(udp_pkt->hdr.src_port);

    // 提交给合适的sock
    udp_t * udp = (udp_t*)udp_find(src_ip, remote_port, dest_ip, local_port);
    if (!udp) {
        dbg_error(DBG_UDP, "no udp for this packet");
        return NET_ERR_UNREACH;
    }

    // 有UDP处理，移去IP包头，重新获得UDP包头
    pktbuf_remove_header(buf, iphdr_size);
    udp_pkt = (udp_pkt_t*)pktbuf_data(buf);
    if (udp_pkt->hdr.checksum) {
        pktbuf_reset_acc(buf);
        if (checksum_peso(dest_ip->a_addr, src_ip->a_addr, NET_PROTOCOL_UDP, buf)) {
            dbg_warning(DBG_UDP, "udp check sum incorrect");
            return NET_ERR_CHKSUM;
        }
    }

    // 检查包的合法性
    udp_pkt = (udp_pkt_t*)(pktbuf_data(buf));
    udp_pkt->hdr.src_port = x_ntohs(udp_pkt->hdr.src_port);
    udp_pkt->hdr.dest_port = x_ntohs(udp_pkt->hdr.dest_port);
    udp_pkt->hdr.total_len = x_ntohs(udp_pkt->hdr.total_len);
    if ((err = is_pkt_ok(udp_pkt, buf->total_size)) <  0) {
        dbg_error(DBG_UDP, "udp packet error");
        return err;
    }
    display_udp_packet(udp_pkt);

    // 填充来源信息
    pktbuf_remove_header(buf, (int)(sizeof(udp_hdr_t) - sizeof(udp_from_t)));
    udp_from_t* from = (udp_from_t *)pktbuf_data(buf);
    from->port = remote_port;
    ipaddr_copy(&from->from, src_ip);

    // 写到接收队列，通知应用程序取
    if (nlist_count(&udp->recv_list) < UDP_MAX_RECV) {
        nlist_insert_last(&udp->recv_list, &buf->node);

        // 信号量通知线程去取包。如果需要由DNS处理，则将由DNS
        sock_wakeup((sock_t *)udp, SOCK_WAIT_READ, NET_ERR_OK);
    } else {
        dbg_warning(DBG_UDP, "queue full, drop pkt");
        pktbuf_free(buf);
    }

    // DNS输入检查
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
    udp_hdr->checksum = checksum_peso(src->a_addr, dest->a_addr, NET_PROTOCOL_UDP, buf);;

    // 交由IP层发送, tos和ttl暂定为固定值
    err = ipv4_out(NET_PROTOCOL_UDP, dest, src, buf);
    if (err < 0) {
        dbg_error(DBG_UDP, "udp out error, err = %d", err);
        return err;
    }

    return err;
}
