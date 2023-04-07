#include "sock.h"
#include "sys.h"
#include "exmsg.h"
#include "dbg.h"
#include "socket.h"
#include "raw.h"

#define SOCKET_MAX_NR       (RAW_MAX_NR)

static x_socket_t socket_tbl[SOCKET_MAX_NR];

// socket - tbl - 0, 1
// (addr_socket - add_tbl) / sizeof(x_socket_t)
static int get_index (x_socket_t * socket) {
    return (int)(socket - socket_tbl);
}

static x_socket_t * get_socket (int idx) {
    if ((idx < 0) || (idx >= SOCKET_MAX_NR)) {
        return (x_socket_t *)0;
    }

    return socket_tbl + idx;
}

static x_socket_t * socket_alloc (void) {
    x_socket_t * s = (x_socket_t *)0;

    for (int i = 0; i < SOCKET_MAX_NR; i++) {
        x_socket_t * curr = socket_tbl + i;
        if (curr->state == SOCKET_STATE_FREE) {
            curr->state = SOCKET_STATE_USED;
            s = curr;
            break;
        }
    }
    return s;
}
static void socket_free (x_socket_t * s) {
    s->state = SOCKET_STATE_FREE;
}

net_err_t socket_init (void) {
    plat_memset(socket_tbl, 0, sizeof(socket_tbl));
    return NET_ERR_OK;
}

net_err_t sock_init (sock_t * sock, int family, int protocol, const sock_ops_t * ops) {
    sock->protocol = protocol;
    sock->family = family;
    sock->ops = ops;

    ipaddr_set_any(&sock->local_ip);
    ipaddr_set_any(&sock->remote_ip);
    sock->local_port = 0;
    sock->remote_port = 0;
    sock->err = NET_ERR_OK;
    sock->rcv_tmo = 0;
    sock->snd_tmo = 0;
    nlist_node_init(&sock->node);
    return NET_ERR_OK;
}

net_err_t sock_create_req_in (struct _func_msg_t * msg) {
    static const struct sock_info_t {
        int protocol;
        sock_t * (*create) (int family, int protocol);
    }sock_tbl[] = {
        [SOCK_RAW] = {.protocol = IPPROTO_ICMP, .create = raw_create,}
    };

    sock_req_t * req = (sock_req_t *)msg->param;
    sock_create_t * param = &req->create;

    x_socket_t * s = socket_alloc();
    if (!s) {
        dbg_error(DBG_SOCKET, "no socket");
        return NET_ERR_MEM;
    }

    if ((param->type < 0) || (param->type >= sizeof(socket_tbl) / sizeof(socket_tbl[0]))) {
        dbg_error(DBG_SOCKET, "create sock fiailed");
        socket_free(s);
        return NET_ERR_PARAM;
    }

    const struct sock_info_t * info = sock_tbl + param->type;
    if (param->protocol == 0) {
        param->protocol = info->protocol;
    }

    sock_t * sock = info->create(param->family, param->protocol);
    if (!sock) {
        dbg_error(DBG_SOCKET, "create sock fiailed");
        socket_free(s);
        return NET_ERR_MEM;
    }

    s->sock = sock;
    req->sockfd = get_index(s);
    return NET_ERR_OK;
}

net_err_t sock_sendto_req_in (struct _func_msg_t * msg) {
    sock_req_t * req = (sock_req_t *)msg->param;

    x_socket_t * s = get_socket(req->sockfd);
    if (!s) {
        dbg_error(DBG_SOCKET, "param error");
        return NET_ERR_PARAM;
    }

    sock_t * sock = s->sock;
    sock_data_t * data = &req->data;

    if (!sock->ops->sendto) {
        dbg_error(DBG_SOCKET, "funtion not imp");
        return NET_ERR_NOT_SUPPORT;
    }

    net_err_t err = sock->ops->sendto(sock, data->buf, data->len, data->flags, 
                            data->addr, data->addr_len, &data->comp_len);
    return err;
}
