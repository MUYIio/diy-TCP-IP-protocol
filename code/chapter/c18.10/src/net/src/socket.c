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
