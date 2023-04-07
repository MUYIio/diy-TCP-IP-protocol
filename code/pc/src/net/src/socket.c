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
#include "dns.h"

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
        exmsg_func_exec(sock_destroy_req_in, &req);
        return -1;
    }

    // 关闭时需要有断开连接的过程，因此需要等
    if (req.wait) {
        // 既然要等，那就等吧。不断等成不成功，最后都进行强制的销毁
        // 特殊的，对于TCP，如果处于TIME-WAIT，则不会主动销毁，而是由其内部定时器自动销毁
        sock_wait_enter(req.wait, req.wait_tmo);
        exmsg_func_exec(sock_destroy_req_in, &req);
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
        if (err == NET_ERR_CLOSE) {
            // 远端主动关闭，收到FIN，返回EOF
            dbg_warning(DBG_SOCKET, "remote cloase");
            return NET_ERR_EOF;
        } else if (err < 0) {
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
        if (err == NET_ERR_CLOSE) {
            // 远端主动关闭，收到FIN，返回EOF
            dbg_warning(DBG_TCP, "remote cloase");
            return NET_ERR_EOF;
        } else if (err < 0) {
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

/**
 * @brief 查询指定域名的IP地址等信息
 * 由于设计上的限制，目前只做一个IP地址的查询
 * @param name 查询的域名
 * @param ret 存储查询结构的结构
 * @param buf 临时缓存，用于存储查询结果信息
 * @param len 临时缓存长
 * @param result 查询结果指针，成功时指向ret, 失败时指向NULL
 * @param err 存储错误码
 */
int x_gethostbyname_r (const char *name,struct x_hostent *ret, char *buf, size_t len,
         struct x_hostent **result, int *err) {
    // 基本的参数检查
    if (!name || !ret || !buf || !len || !result) {
        dbg_error(DBG_SOCKET, "param error");
        *err = NET_ERR_PARAM;
        return -1;
    }

    if (len < (plat_strlen(name) + sizeof(hostent_extra_t))) {
        dbg_error(DBG_SOCKET, "size too samll: %d", len);
        *err = NET_ERR_PARAM;
        return -1;
    }

    // err可能为空。如果每次在后边判断就比较麻烦，因此在这里使用内部临时的值
    int internal_err;
    if (!err) {
        err = &internal_err;
    }

    size_t name_len = plat_strlen(name);            // 不用算上最后的'\0'
    if (len < (sizeof(hostent_extra_t) + name_len)) { // 因为sizeof这里有计算进去
        dbg_error(DBG_SOCKET, "buf too small");
        *err = NET_ERR_PARAM;
        return -1;
    }

    // 这里后续如果等结果，会将dns_req局部变量挂载到整个等待链接的内部
    // 如此以来，直接删除当前任务就可能导致链表中的req失效，造成影响
    // 因此后续这里可以做成动态分配的方式

    // 发送查询请求，查询完成后调用调函数取结果
    dns_req_t * dns_req = dns_alloc_req();
    plat_strncpy(dns_req->domain_name, name, DNS_DOMAIN_NAME_MAX);
    ipaddr_set_any(&dns_req->ipaddr);
    dns_req->wait_sem = SYS_SEM_INVALID;
    net_err_t e = exmsg_func_exec(dns_req_in, dns_req);
    if (e < 0) {
        dbg_error(DBG_SOCKET, "get host failed:", e);
        *err = e;
        goto dns_req_error;
    }

    // 成功，有可能查到了正确的结果，也可能是需要等待
    // 等待结果，DNS内部会超时处理，这里无需再超时
    if ((dns_req->wait_sem != SYS_SEM_INVALID) && sys_sem_wait(dns_req->wait_sem, 0) < 0) {
        dbg_error(DBG_SOCKET, "wait sem failed.");
        *err = NET_ERR_TMO;
        goto dns_req_error;
    }

    if (dns_req->err < 0) {
        dbg_error(DBG_SOCKET, "dns resolve failed.");
        *err = dns_req->err;
        goto dns_req_error;
    }
    // 处理结果
    hostent_extra_t * extra = (hostent_extra_t *)buf;
    extra->addr = dns_req->ipaddr.q_addr;
    plat_strncpy(extra->name, name, len - sizeof(hostent_extra_t));  // 已经将'\0'算进去了
    buf[len -1] = '\0';      // 加上字符串结束符

    ret->h_name = extra->name;
    ret->h_aliases = (char **)0;
    ret->h_addrtype = AF_INET;  // IPv4地址
    ret->h_length = 4;          // IPv4，4字节地址
    ret->h_addr_list = (char **)extra->addr_tbl;
    ret->h_addr_list[0] = (char *)&extra->addr;
    ret->h_addr_list[1] = (char *)0;

    // 设置返回情况
    *result = ret;
    *err = 0;
    dns_free_req(dns_req);
    return 0;
dns_req_error:
    dns_free_req(dns_req);
    return -1;
}

/**
 * @brief 接受来自远端的连接
 */
int x_accept(int sockfd, struct x_sockaddr* addr, x_socklen_t* len) {
    // 参数检查
    if (!addr || !len) {
        dbg_error(DBG_SOCKET, "addr len error");
        return -1;
    }

     // 将要发的数据填充进消息体，请求发送
    while (1) {
        sock_req_t req;
        req.sockfd = sockfd;
        req.accept.addr = addr;
        req.accept.len = len;
        req.accept.client = -1;
        net_err_t err = exmsg_func_exec(sock_accept_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_TCP, "setopt:", err);
            return -1;
        }

        // 是否取得结果
        if (req.accept.client >= 0) {
            dbg_info(DBG_TCP, "get new connection");
            return req.accept.client;
        }

        // 等待连续建立成功
        if (req.wait && ((err = sock_wait_enter(req.wait, req.wait_tmo)) < NET_ERR_OK)) {
            dbg_error(DBG_TCP, "connect failed %d.", err);
            return -1;
        }
    }
}

/**
 * @brief 进入监听状态
 */
int x_listen(int sockfd, int backlog) {
     // 将要发的数据填充进消息体，请求发送
    sock_req_t req;
    req.wait = 0;
    req.sockfd = sockfd;
    req.listen.backlog = backlog;
    net_err_t err = exmsg_func_exec(sock_listen_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_TCP, "setopt:", err);
        return -1;
    }

    return 0;
}

/**
 * @brief 写数据
 */
ssize_t x_write(int sid, const void* buf, size_t len) {
    return x_send(sid, buf, len, 0);
}

/**
 * @brief 读取数据
 */
ssize_t x_read(int sid, void* buf, size_t len) {
    return x_recv(sid, buf, len, 0);
}
