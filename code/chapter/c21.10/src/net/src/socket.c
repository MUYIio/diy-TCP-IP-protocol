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
    req.wait = 0;
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
 * @brief 关闭socket
 */
int x_close(int sockfd) {
    // 发起请求
    sock_req_t req;
    req.wait = 0;
    req.sockfd = sockfd;
    net_err_t err = exmsg_func_exec(sock_close_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "try close failed %d, force delete.", err);
        return -1;
    }

    // 关闭时需要有断开连接的过程，因此需要等
    if (req.wait) {
        // 既然要等，那就等吧。不断等成不成功，最后都进行强制的销毁
        // 特殊的，对于TCP，如果处于TIME-WAIT，则不会主动销毁，而是由其内部定时器自动销毁
        sock_wait_enter(req.wait, req.wait_tmo);
    }

    // TODO: 这里还有些工作未完成，等到TCP部分时再处理
    return 0;
}

/**
 * @brief 发送数据包
 * 每次发送数据，并不一定全部发完，而是能发多少就发多少
 * 另外，数据只是写入发送缓存。至于什么时候发，对方能不能及时收到这不确定
 * 所以执行完send函数以后，并不保证数据必然被对方接收到
 */
ssize_t x_send(int sockfd, const void* buf, size_t len, int flags) {
    ssize_t send_size = 0;
    uint8_t * start = (uint8_t *)buf;
    while (len) {
        // 将要发的数据填充进消息体，请求发送
        sock_req_t req;
        req.wait = 0;
        req.sockfd = sockfd;
        req.data.buf = start;
        req.data.len = len;
        req.data.flags = flags;
        req.data.comp_len = 0;
        net_err_t err = exmsg_func_exec(sock_send_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "write failed.");
            return -1;
        }

        // 等待数据写入发送缓存中。注意，只是写入缓存，并不一定发送垸 成
        if (req.wait && ((err = sock_wait_enter(req.wait, req.wait_tmo)) < NET_ERR_OK)) {
            dbg_error(DBG_SOCKET, "send failed %d.", err);
            return -1;
        }

        len -= req.data.comp_len;
        send_size += (ssize_t)req.data.comp_len;
        start += req.data.comp_len;
    }

    return send_size;
}

/**
 * @brief 接收数据
 */
ssize_t x_recv(int sockfd, void* buf, size_t len, int flags) {
    // 循环读取，直到读取到数据
    while (1) {
        // 填充消息，发出读请求
        sock_req_t req;
        req.wait = 0;
        req.sockfd = sockfd;
        req.data.buf = buf;
        req.data.len = len;
        req.data.comp_len = 0;
        net_err_t err = exmsg_func_exec(sock_recv_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "rcv failed.:", err);
            return -1;
        }

        // 当读取到数据时，结束读取过程
        if (req.data.comp_len) {
            return (ssize_t)req.data.comp_len;
        }

        // 等待连接建立, 即进入已经建立连接状态，或者中间出现错误
        err = sock_wait_enter(req.wait, req.wait_tmo);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "recv failed %d.", err);
            return -1;
        }
    }
}

/**
 * @brief 连接到远程主机
 */
int x_connect(int sockfd, const struct x_sockaddr* addr, x_socklen_t len) {
    // 参数检查
    if ((len != sizeof(struct x_sockaddr)) || !addr) {
        dbg_error(DBG_SOCKET, "addr error");
        return -1;
    }

    if (addr->sa_family != AF_INET) {
        dbg_error(DBG_SOCKET, "family error");
        return -1;
    }

    // 地址和端口号不可能为空
    const struct x_sockaddr_in* addr_in = (const struct x_sockaddr_in*)addr;
    if ((addr_in->sin_addr.s_addr == INADDR_ANY) || !addr_in->sin_port) {
        dbg_error(DBG_SOCKET, "ip or port is empty");
        return -1;
    }

   // 调用底层的连接处理函数
    sock_req_t req;
    req.wait = 0;
    req.sockfd = sockfd;
    req.conn.addr = addr;
    req.conn.len = len;
    net_err_t err = exmsg_func_exec(sock_connect_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "try connect failed: %d", err);
        return -1;
    }

    if (req.wait && ((err = sock_wait_enter(req.wait, req.wait_tmo)) < NET_ERR_OK)) {
        dbg_error(DBG_SOCKET, "connect failed %d.", err);
        return -1;
    }

    return 0;
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
        req.wait = 0;
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

        // 等待数据写入发送缓存中。注意，只是写入缓存，并不一定发送垸 成
        if (req.wait && ((err = sock_wait_enter(req.wait, req.wait_tmo)) < NET_ERR_OK)) {
            dbg_error(DBG_SOCKET, "send failed %d.", err);
            return -1;
        }

        size -= req.data.comp_len;
        send_size += (ssize_t)req.data.comp_len;
        start += req.data.comp_len;
    }

    return send_size;
}

/**
 * @brief 接收数据包消息处理
 */
ssize_t x_recvfrom(int sockfd, void* buf, size_t size, int flags, struct x_sockaddr* addr, x_socklen_t* len) {
    if (!size || !len || !addr) {
        dbg_error(DBG_SOCKET, "addr size or len error");
        return -1;
    }

    // 循环读取，直到读取到数据
    while (1) {
        // 填充消息，发出读请求
        sock_req_t req;
        req.wait = 0;
        req.sockfd = sockfd;
        req.data.buf = buf;
        req.data.len = size;
        req.data.comp_len = 0;
        req.data.addr = addr;
        req.data.addr_len = len;
        net_err_t err = exmsg_func_exec(sock_recvfrom_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "connect failed:", err);
            return -1;
        }

        // 当读取到数据时，结束读取过程
        if (req.data.comp_len) {
            return (ssize_t)req.data.comp_len;
        }

        err = sock_wait_enter(req.wait, req.wait_tmo);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "recv failed %d.", err);
            return -1;
        }

    }
}

/**
 * @brief 设置套接字的选项
 */
int x_setsockopt(int sockfd, int level, int optname, const char * optval, int optlen) {
    if (!optval || !optlen) {
        dbg_error(DBG_SOCKET, "param error", NET_ERR_PARAM);
        return -1;
    }

    // 将要发的数据填充进消息体，请求发送
    sock_req_t req;
    req.wait = 0;
    req.sockfd = sockfd;
    req.opt.level = level;
    req.opt.optname = optname;
    req.opt.optval = optval;
    req.opt.optlen = optlen;
    net_err_t err = exmsg_func_exec(sock_setsockopt_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "setopt:", err);
        return -1;
    }

    return 0;
}

/**
 * @brief 绑定本地地址
 */
int x_bind(int sockfd, const struct x_sockaddr* addr, x_socklen_t len) {
    // 参数检查
    if ((len != sizeof(struct x_sockaddr)) || !addr) {
        dbg_error(DBG_SOCKET, "addr len error");
        return -1;
    }

    if (addr->sa_family != AF_INET) {
        dbg_error(DBG_SOCKET, "family error");
        return -1;
    }

     // 将要发的数据填充进消息体，请求发送
    sock_req_t req;
    req.wait = 0;
    req.sockfd = sockfd;
    req.bind.addr = addr;
    req.bind.len = len;
    net_err_t err = exmsg_func_exec(sock_bind_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "setopt:", err);
        return -1;
    }

    return 0;
}
