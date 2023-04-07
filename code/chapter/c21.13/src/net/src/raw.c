/**
 * @file raw.c
 * @author lishutong(527676163@qq.com)
 * @brief 原始套接字
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 * 
 * 给应用层使用的IP控制块, 用于收发IP层的数据报，可以实现例如ping等功能
 */
#include "raw.h"
#include "mblock.h"
#include "dbg.h"
#include "sock.h"
#include "socket.h"
#include "ipv4.h"

static raw_t raw_tbl[RAW_MAX_NR];           // raw控制块数组
static mblock_t raw_mblock;                 // 空闲raw控制块链表
static nlist_t raw_list;                     // 已绑定的控制块链表

#if DBG_DISP_ENABLED(DBG_RAW)
static void display_raw_list (void) {
    plat_printf("\n--- raw list\n --- ");

    int idx = 0;
    nlist_node_t * node;

    nlist_for_each(node, &raw_list) {
        raw_t * raw = (raw_t *)nlist_entry(node, sock_t, node);
        plat_printf("[%d]\n", idx++);
        dump_ip_buf("\tlocal:", (const uint8_t *)&raw->base.local_ip.a_addr);
        dump_ip_buf("\tremote:", (const uint8_t *)&raw->base.remote_ip.a_addr);
    }
}
#else
#define display_raw_list()
#endif

/**
 * @brief 绑定地址
 */
net_err_t raw_bind(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len) {
    net_err_t err = sock_bind(sock, addr, len);
    display_raw_list();
    return err;
}

/**
 * @breif 连接到远程主机
 */
net_err_t raw_connect(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len) {
    net_err_t err = sock_connect(sock, addr, len);
    display_raw_list();
    return err;
}

/**
 * @brief 发送一个IP数据包
 */
static net_err_t raw_sendto (struct _sock_t * sock, const void* buf, size_t len, int flags, const struct x_sockaddr* dest,
            x_socklen_t dest_len, ssize_t * result_len) {
    // 如果已经连接，则检查IP是否一致，不一致则报错
    ipaddr_t dest_ip;
    struct x_sockaddr_in* addr = (struct x_sockaddr_in*)dest;
    ipaddr_from_buf(&dest_ip, addr->sin_addr.addr_array);
    if (!ipaddr_is_any(&sock->remote_ip) && !ipaddr_is_equal(&dest_ip, &sock->remote_ip)) {
        dbg_error(DBG_RAW, "dest is incorrect");
        return NET_ERR_CONNECTED;
    }

    // 分配缓存空间
    pktbuf_t* pktbuf = pktbuf_alloc((int)len);
    if (!pktbuf) {
        dbg_error(DBG_RAW, "no buffer");
        return NET_ERR_MEM;
    }

    // 数据拷贝过去
    net_err_t err = pktbuf_write(pktbuf, (uint8_t *)buf, (int)len);
    if (sock->err < 0) {
        dbg_error(DBG_RAW, "copy data error");
        goto end_sendto;
    }

    // 通过IP层发送出去
    err = ipv4_out(sock->protocol, &dest_ip, &sock->local_ip, pktbuf);
    if (err < 0) {
        dbg_error(DBG_RAW, "send error");
        goto end_sendto;
    }

    *result_len = (ssize_t)len;
    return NET_ERR_OK;
end_sendto:
    pktbuf_free(pktbuf);
    return err;
}

/**
 * @brief 读取一些数据
 */
static net_err_t raw_recvfrom (struct _sock_t* sock, void* buf, size_t len, int flags,
            struct x_sockaddr* src, x_socklen_t * addr_len, ssize_t * result_len) {
    raw_t * raw = (raw_t *)sock;

    nlist_node_t * first = nlist_remove_first(&raw->recv_list);
    if (!first) {
        *result_len = 0;
        // 后续任务需要继续等待
        return NET_ERR_NEED_WAIT;
    }

    pktbuf_t* pktbuf = nlist_entry(first, pktbuf_t, node);
    dbg_assert(pktbuf != (pktbuf_t *)0, "pktbuf error");

    // 将IP地址从包中拷贝到src结构中
    ipv4_hdr_t* iphdr = (ipv4_hdr_t*)pktbuf_data(pktbuf);
    struct x_sockaddr_in* addr = (struct x_sockaddr_in*)src;
    plat_memset(addr, 0, sizeof(struct x_sockaddr));
    addr->sin_family = AF_INET;
    addr->sin_port = 0;
    plat_memcpy(&addr->sin_addr, iphdr->src_ip, IPV4_ADDR_SIZE);

    // 从包中读取数据
    int size = (pktbuf->total_size > (int)len) ? (int)len : pktbuf->total_size;
    pktbuf_reset_acc(pktbuf);
    net_err_t err= pktbuf_read(pktbuf, buf, size);
    if (err < 0) {
        pktbuf_free(pktbuf);
        dbg_error(DBG_RAW, "pktbuf read error");
        return err;
    }

    pktbuf_free(pktbuf);

    *result_len = size;
    return NET_ERR_OK;
}

/**
 * @brief 关闭连接
 * 关闭比较简单，直接移除，不需要再通信
 */
net_err_t raw_close(sock_t * sock) {
    raw_t * raw = (raw_t *)sock;

    // 从链表中移除
    nlist_remove(&raw_list, &sock->node);

    // 释放所有数据包
	nlist_node_t* node;
	while ((node = nlist_remove_first(&raw->recv_list))) {
		pktbuf_t* buf = nlist_entry(node, pktbuf_t, node);
		pktbuf_free(buf);
	}

    // sock关闭并释放掉
    sock_uninit(sock);
    mblock_free(&raw_mblock, sock);

    display_raw_list();
    return NET_ERR_OK;
}

/**
 * @brief 创建原始套接字操作结构
 */
sock_t* raw_create(int family, int protocol) {
     // raw特有的sock操作列表
    static const sock_ops_t raw_ops = {
         .sendto = raw_sendto,
         .recvfrom = raw_recvfrom,
        .setopt = sock_setopt,
        .close = raw_close,
        .connect = raw_connect,
        .bind = raw_bind,
        .send = sock_send,      // 使用基础sock默认提供的
        .recv = sock_recv,
    };

    // 分配一个raw sock结构
    raw_t* raw = mblock_alloc(&raw_mblock, -1);
    if (!raw) {
        dbg_error(DBG_RAW, "no raw sock");
        return (sock_t*)0;
    }

    // 初始化通用的sock结构部分
    net_err_t err = sock_init((sock_t*)raw, family, protocol, &raw_ops);
    if (err < 0) {
        dbg_error(DBG_RAW, "create raw failed.");
        mblock_free(&raw_mblock, raw);
        return (sock_t*)0;
    }
    nlist_init(&raw->recv_list);

    // 接收时需要等待
    raw->base.rcv_wait = &raw->rcv_wait;
    if (sock_wait_init(raw->base.rcv_wait) < 0) {
        dbg_error(DBG_RAW, "create rcv.wait failed");
        goto create_failed;
    }

    // 插入全局队列中
    nlist_insert_last(&raw_list, &raw->base.node);

    display_raw_list();
    return (sock_t *)raw;
create_failed:
    sock_uninit((sock_t *)raw);
    return (sock_t *)0;
}

/**
 * @brief RAW模块初始化
 */
net_err_t raw_init(void) {
    dbg_info(DBG_RAW, "raw init.");

    mblock_init(&raw_mblock, raw_tbl, sizeof(raw_t), RAW_MAX_NR, NLOCKER_NONE);
    nlist_init(&raw_list);

    dbg_info(DBG_RAW, "init done.");
    return NET_ERR_OK;
}

/**
 * @brief 查找匹配的raw控制块
 * 下面的算法比较简单，先找到哪个匹配即使用哪个，不会去查更全匹配的项
 */
static raw_t * raw_find (ipaddr_t * src, ipaddr_t * dest, int protocol) {
    nlist_node_t* node;
    raw_t * found = (raw_t *)0;

    nlist_for_each(node, &raw_list) {
        raw_t* raw = (raw_t *)nlist_entry(node, sock_t, node);

        // todo: 这里要不要加锁呢？
        // 指定了协议，但是协议与收到的不同，不匹配
        if (raw->base.protocol && (raw->base.protocol != protocol)) {
            continue;
        }

        // 指定了本地IP，但是收到的IP不是自己的这个，不匹配
        if (!ipaddr_is_any(&raw->base.local_ip) && !ipaddr_is_equal(&raw->base.local_ip, dest)) {
            continue;
        }

        // 指定了远端IP，但是收到的IP不是对方的，不匹配
        if (!ipaddr_is_any(&raw->base.remote_ip) && !ipaddr_is_equal(&raw->base.remote_ip, src)) {
            continue;
        }

        found = raw;
        break;
    }

    return found;
}

/**
 * @brief RAW数据包输入处理, 输入的包包含了IP包头，并且包头是连续的
 */
net_err_t raw_in(pktbuf_t* pktbuf) {
    ipv4_hdr_t* iphdr = (ipv4_hdr_t*)pktbuf_data(pktbuf);
    net_err_t err = NET_ERR_UNREACH;

    ipaddr_t src, dest;
    ipaddr_from_buf(&dest, iphdr->dest_ip);
    ipaddr_from_buf(&src, iphdr->src_ip);

    // 找能处理的控制块
    raw_t * raw = raw_find(&src, &dest, iphdr->protocol);
    if (raw == (raw_t *)0) {
        dbg_warning(DBG_RAW, "no raw for this packet");
        return NET_ERR_UNREACH;
    }

    // 将数据包发送给raw，并通知应用程序去取，只发给一个应用
    if (nlist_count(&raw->recv_list) < RAW_MAX_RECV) {
        nlist_insert_last(&raw->recv_list, &pktbuf->node);

        // 信号量通知线程去取包
        sock_wakeup((sock_t *)raw, SOCK_WAIT_READ, NET_ERR_OK);
    } else {
        // 没有可处理的，释放掉
        pktbuf_free(pktbuf);
    }
    return NET_ERR_OK;
}
