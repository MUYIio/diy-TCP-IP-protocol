#include "sock.h"
#include "socket.h"
#include "exmsg.h"
#include "dbg.h"

int x_socket(int family, int type, int protocol) {
    sock_req_t req;

    req.sockfd = -1;
    req.create.family = family;
    req.create.protocol = protocol;
    req.create.type = type;

    net_err_t err = exmsg_func_exec(sock_create_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "create  socket failed.");
        return -1;
    }

    return req.sockfd;
}

ssize_t x_sendto(int s, const void * buf, size_t len, int flags,  
                const struct x_sockaddr * dest, x_socklen_t dest_len) {
    if (!buf || !len) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }

    if ((dest->sin_family != AF_INET) || (dest_len != sizeof(struct x_sockaddr_in))) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }

    ssize_t send_size = 0;
    uint8_t * start = (uint8_t *)buf;
    while (len > 0) {
        sock_req_t req;
        req.sockfd = s;
        req.data.buf = start;
        req.data.len = len;
        req.data.flags = 0;
        req.data.addr = (struct x_sockaddr *)dest;
        req.data.addr_len = dest_len;
        req.data.comp_len = 0;

        net_err_t err = exmsg_func_exec(sock_sendto_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "send  socket failed.");
            return -1;
        }

        len -= req.data.comp_len;
        start += req.data.comp_len;
        send_size += (ssize_t)req.data.comp_len;
    }

    return send_size;
}
