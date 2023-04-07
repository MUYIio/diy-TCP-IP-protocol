#include "sock.h"
#include "socket.h"
#include "exmsg.h"
#include "dbg.h"

int x_socket(int family, int type, int protocol) {
    sock_req_t req;

    req.wait = (sock_wait_t *)0;
    req.wait_tmo = 0;
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
        req.wait = (sock_wait_t *)0;
        req.wait_tmo = 0;
        req.data.buf = start;
        req.data.len = len;
        req.data.flags = 0;
        req.data.addr = (struct x_sockaddr *)dest;
        req.data.addr_len = &dest_len;
        req.data.comp_len = 0;

        net_err_t err = exmsg_func_exec(sock_sendto_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "send  socket failed.");
            return -1;
        }

        if (req.wait && ((err == sock_wait_enter(req.wait, req.wait_tmo)) < 0)) {
            dbg_error(DBG_SOCKET, "send failed.");
            return -1;
        }
        len -= req.data.comp_len;
        start += req.data.comp_len;
        send_size += (ssize_t)req.data.comp_len;
    }

    return send_size;
}

ssize_t x_send(int s, const void * buf, size_t len, int flags) {
    if (!buf || !len) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }

    ssize_t send_size = 0;
    uint8_t * start = (uint8_t *)buf;
    while (len > 0) {
        sock_req_t req;
        req.sockfd = s;
        req.wait = (sock_wait_t *)0;
        req.wait_tmo = 0;
        req.data.buf = start;
        req.data.len = len;
        req.data.flags = 0;
        req.data.comp_len = 0;

        net_err_t err = exmsg_func_exec(sock_send_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "send  socket failed.");
            return -1;
        }

        if (req.wait && ((err == sock_wait_enter(req.wait, req.wait_tmo)) < 0)) {
            dbg_error(DBG_SOCKET, "send failed.");
            return -1;
        }
        len -= req.data.comp_len;
        start += req.data.comp_len;
        send_size += (ssize_t)req.data.comp_len;
    }

    return send_size;
}

ssize_t x_recvfrom(int s, void * buf, size_t len, int flags,  
                const struct x_sockaddr * src, x_socklen_t * src_len) {
    if (!buf || !len || !src) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }           

    while (1) {
        sock_req_t req;
        req.wait = (sock_wait_t *)0;
        req.wait_tmo = 0;
        req.sockfd = s;
        req.data.buf = buf;
        req.data.len = len;
        req.data.flags = 0;
        req.data.addr = (struct x_sockaddr *)src;
        req.data.addr_len = src_len;
        req.data.comp_len = 0;

        net_err_t err = exmsg_func_exec(sock_recvfrom_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "recv  socket failed.");
            return -1;
        }

        if (req.data.comp_len) {
            return (ssize_t)req.data.comp_len;
        }

        err = sock_wait_enter(req.wait, req.wait_tmo);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "recv failed.");
            return -1;
        }
    }
}

ssize_t x_recv(int s, void * buf, size_t len, int flags) {
    if (!buf || !len) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }           

    while (1) {
        sock_req_t req;
        req.wait = (sock_wait_t *)0;
        req.wait_tmo = 0;
        req.sockfd = s;
        req.data.buf = buf;
        req.data.len = len;
        req.data.flags = 0;
        req.data.comp_len = 0;

        net_err_t err = exmsg_func_exec(sock_recv_req_in, &req);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "recv  socket failed.");
            return -1;
        }

        if (req.data.comp_len) {
            return (ssize_t)req.data.comp_len;
        }

        err = sock_wait_enter(req.wait, req.wait_tmo);
        if (err < 0) {
            dbg_error(DBG_SOCKET, "recv failed.");
            return -1;
        }
    }
}


int x_setsockopt(int s, int level, int optname, const char * optval, int len) {
    if (!optval || !len) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }


    sock_req_t req;
    req.wait = (sock_wait_t *)0;
    req.wait_tmo = 0;
    req.sockfd = s;
    req.opt.level = level;
    req.opt.optname = optname;
    req.opt.optval = optval;
    req.opt.len = len;

    net_err_t err = exmsg_func_exec(sock_setsockopt_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "create  socket failed.");
        return -1;
    }

    return 0;    
}

int x_close (int s) {
    sock_req_t req;

    req.wait = (sock_wait_t *)0;
    req.wait_tmo = 0;
    req.sockfd = s;

    net_err_t err = exmsg_func_exec(sock_close_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "close  socket failed.");
        return -1;
    }

    return 0;
}

int x_connect(int s, const struct x_sockaddr * addr, x_socklen_t len) {
    if ((!addr) || (len != sizeof(struct x_sockaddr)) || (s < 0)) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }

    if (addr->sin_family != AF_INET) {
        dbg_error(DBG_SOCKET, "family error");
        return -1;
    }

    sock_req_t req;
    req.wait = 0;
    req.sockfd = s;
    req.conn.addr = (struct x_sockaddr *)addr;
    req.conn.addr_len = len;
    net_err_t err = exmsg_func_exec(sock_connect_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "conn failed.");
        return -1;
    }
    return 0;
}

int x_bind(int s, const struct x_sockaddr * addr, x_socklen_t len) {
    if ((!addr) || (len != sizeof(struct x_sockaddr)) || (s < 0)) {
        dbg_error(DBG_SOCKET, "param error");
        return -1;
    }

    if (addr->sin_family != AF_INET) {
        dbg_error(DBG_SOCKET, "family error");
        return -1;
    }

    sock_req_t req;
    req.wait = 0;
    req.sockfd = s;
    req.bind.addr = (struct x_sockaddr *)addr;
    req.bind.addr_len = len;
    net_err_t err = exmsg_func_exec(sock_bind_req_in, &req);
    if (err < 0) {
        dbg_error(DBG_SOCKET, "bind failed.");
        return -1;
    }
    return 0;    
}
