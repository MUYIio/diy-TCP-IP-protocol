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

static raw_t raw_tbl[RAW_MAX_NR];           // raw控制块数组
static mblock_t raw_mblock;                 // 空闲raw控制块链表
static nlist_t raw_list;                     // 已绑定的控制块链表


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
    err = ipv4_out(sock->protocol, &dest_ip, &netif_get_default()->ipaddr, pktbuf);
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

    *result_len = 0;
    return NET_ERR_NEED_WAIT;
}

/**
 * @brief 创建原始套接字操作结构
 */
sock_t* raw_create(int family, int protocol) {
     // raw特有的sock操作列表
    static const sock_ops_t raw_ops = {
         .sendto = raw_sendto,
         .recvfrom = raw_recvfrom,
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

    // 插入全局队列中
    nlist_insert_last(&raw_list, &raw->base.node);

    return (sock_t *)raw;
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
