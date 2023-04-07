/**
 * @file socket.c
 * @author lishutong(527676163@qq.com)
 * @brief 套接字接口类型
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 * 
 * 主要负责完成套接字接口函数的实现，在内部对参数做了一些基本的检查。具体的各项
 * 工作会由更底层的sock结构完成。每种不同类型sock完成实现特定的操作接口ops
 */
#include "socket.h"
#include "mblock.h"
#include "dbg.h"
#include "sock.h"

/**
 * @brief 创建一个socket套接字
 */
int x_socket(int family, int type, int protocol) {
    // 发消息给sock层，创建一个sock
    sock_req_t req;
    req.sockfd = -1;
    req.create.family = family;
    req.create.type = type;
    req.create.protocol = protocol;
    net_err_t err = exmsg_func_exec(sock_create_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "create sock failed: %d.", err);
        return -1;
    }

    return req.sockfd;
}

/**
 * @brief 发送数据包到指定的目的地
 */
ssize_t x_sendto(int sockfd, const void* buf, size_t size, int flags, const struct x_sockaddr* dest, x_socklen_t addr_len) {
    // 参数检查
    if ((addr_len != sizeof(struct x_sockaddr)) || !size) {
        dbg_error(DBG_SOCKET, "addr size or len error");
        return -1;
    }
    if (dest->sa_family != AF_INET) {
        dbg_error(DBG_SOCKET, "family error");
        return -1;
    }

    ssize_t send_size = 0;
    uint8_t * start = (uint8_t *)buf;
    while (size) {
        // 将要发的数据填充进消息体，请求发送
        sock_req_t req;
        req.sockfd = sockfd;
        req.data.buf = start;
        req.data.len = size;
        req.data.flags = flags;
        req.data.addr = (struct x_sockaddr* )dest;
        req.data.addr_len = &addr_len;
        req.data.comp_len = 0;
        net_err_t err = exmsg_func_exec(sock_sendto_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "write failed.");
            return -1;
        }

        size -= req.data.comp_len;
        send_size += (ssize_t)req.data.comp_len;
        start += req.data.comp_len;
    }

    return send_size;
}
