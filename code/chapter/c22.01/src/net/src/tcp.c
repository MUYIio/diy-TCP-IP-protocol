/**
 * @file tcp.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-11-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "tcp.h"
#include "tools.h"
#include "protocol.h"
#include "ipv4.h"
#include "mblock.h"
#include "dbg.h"
#include "exmsg.h"
#include "socket.h"
#include "timer.h"
#include "tcp_out.h"
#include "tcp_state.h"

static tcp_t tcp_tbl[TCP_MAX_NR];           // tcp控制块数组
static mblock_t tcp_mblock;                 // 空闲TCP控制块链表
static nlist_t tcp_list;                     // 已创建的控制块链表

#if DBG_DISP_ENABLED(DBG_TCP)
/**
 * @brief 显示TCP的状态
 */
void tcp_show_info (char * msg, tcp_t * tcp) {
    plat_printf("%s: %s\n", msg, (tcp->state < TCP_STATE_MAX) ? tcp_state_name(tcp->state) : "UNKNOWN");
    plat_printf("    local port: %u, remote port: %u\n", tcp->base.local_port, tcp->base.remote_port);
    plat_printf("    snd.una: %u, snd.nxt: %u\n", tcp->snd.una, tcp->snd.nxt);
}

void tcp_display_pkt (char * msg, tcp_hdr_t * tcp_hdr, pktbuf_t * buf) {
    plat_printf("%s\n", msg);
    plat_printf("    sport: %u, dport: %u\n", tcp_hdr->sport, tcp_hdr->dport);
    plat_printf("    seq: %u, ack: %u, win: %d\n", tcp_hdr->seq, tcp_hdr->ack, tcp_hdr->win);
    plat_printf("    flags:");
    if (tcp_hdr->f_syn) {
        plat_printf(" syn");
    }
    if (tcp_hdr->f_rst) {
        plat_printf(" rst");
    }
    if (tcp_hdr->f_ack) {
        plat_printf(" ack");
    }
    if (tcp_hdr->f_psh) {
        plat_printf(" push");
    }
    if (tcp_hdr->f_fin) {
        plat_printf(" fin");
    }

    plat_printf("\n    len=%d", buf->total_size - tcp_hdr_size(tcp_hdr));
    plat_printf("\n");
}

void tcp_show_list (void) {
    plat_printf("-------- tcp list -----\n");

    nlist_node_t * node;
    nlist_for_each(node, &tcp_list) {
        tcp_t * tcp = (tcp_t *)nlist_entry(node, sock_t, node);

        tcp_show_info("", tcp);
    }
}
#endif

/**
 * @brief 分配一个有效的TCP端口
 * 下面的算法中，端口号的选择从1024开始递增，但实际不同系统的端口分配看起来是有一定随意性的
 * 也许可以加一点点随机的处理？
 */
int tcp_alloc_port(void) {
#if 1 // NET_DBG
    // 调用试用，以便每次启动调试时使用的端口都不同，wireshark中显示更干净
    // 并且不会与上次的连接重复，避免对本次连接造成干扰。并不严格的处理，只提供随机处理
    srand((unsigned int)time(NULL));
    int search_idx = rand() % 1000 + NET_PORT_DYN_START;
#else
    static int search_idx = NET_PORT_DYN_START;  // 搜索起点
#endif
    // 遍历所有动态端口
    for (int i = NET_PORT_DYN_START; i < NET_PORT_DYN_END; i++) {
        // 看看是否有使用，如果没有使用则返回；否则继续
        nlist_node_t* node;
        nlist_for_each(node, &tcp_list) {
            sock_t* sock = nlist_entry(node, sock_t, node);
            if (sock->local_port == search_idx) {
                // 发现有使用，跳出继续比较
                break;
            }
        }

        // 增加索引
        int port = search_idx++;
        if (search_idx >= NET_PORT_DYN_END) {
            search_idx = NET_PORT_DYN_START;
        }

        // 没有使用，跳出使用该端口结束查询
        if (!node) {
            return port;
        }
    }
    return -1;
}

/**
 * @brief 根据目的IP和端口号地址对，找到对应的sock块
 * tcp链表只有三种类型的块，一是完全匹配的；二是只有指定了本地端口的，三是同时指定了本地端口和IP的
 */
sock_t* tcp_find(ipaddr_t * local_ip, uint16_t local_port, ipaddr_t * remote_ip, uint16_t remote_port) {
    sock_t* match = (sock_t*)0;

    // 遍历整个列表, 找到对应的sock块
    // 优先找完全匹配的项，其次找处理监听匹配的项
    nlist_node_t* node;
    nlist_for_each(node, &tcp_list) {
        sock_t* s = nlist_entry(node, sock_t, node);

        // 地址对完全匹配，即是想要的项
        if ((s->local_port == local_port) &&
            ipaddr_is_equal(&s->remote_ip, remote_ip) && (s->remote_port == remote_port)) {
            if (ipaddr_is_any(&s->local_ip)) {
                return s;
            } else if (ipaddr_is_equal(&s->local_ip, local_ip)) {
                return s;
            }
        }
    }

    // 当没有完全匹配项时，使用部分目的端口匹配的项
    return (sock_t*)match;
}

/**
 * @brief 分配一个TCP，先尝试分配空闲的，然后是重用处于time_wait的
 */
static tcp_t * tcp_get_free (int wait) {
    // 分配一个tcp sock结构
    tcp_t* tcp = (tcp_t*)mblock_alloc(&tcp_mblock, wait ? 0 : -1);
    if (!tcp) {
        return (tcp_t *)0;
    }

    return tcp;
}

/**
 * @brief 分配一个TCP块，该控制块用于用于主动打开或者被动打开
 * 对于处理listen状态的TCP，要避免锁住
 */
static tcp_t* tcp_alloc(int wait, int family, int protocol) {
    static const sock_ops_t tcp_ops = {
        .connect = tcp_connect,
        .close = tcp_close,
    };

    // 分配一个tcp sock结构
    tcp_t* tcp = tcp_get_free(wait);
    if (!tcp) {
        dbg_error(DBG_TCP, "no tcp sock");
        return (tcp_t*)0;
    }
    plat_memset(tcp, 0, sizeof(tcp_t));

    // 基础部分
    net_err_t err = sock_init((sock_t*)tcp, family, protocol, &tcp_ops);
    if (err < 0) {
        dbg_error(DBG_TCP, "create failed.");
        mblock_free(&tcp_mblock, tcp);
        return (tcp_t*)0;
    }
    tcp->state = TCP_STATE_CLOSED;

    // 发送部分初始化，缓存不必初始化，在建立连接时再做
    tcp->snd.una = tcp->snd.nxt = tcp->snd.iss = 0;

    // 接收部分初始化，缓存不必初始化，在建立连接时再做
    tcp->rcv.nxt = tcp->rcv.iss = 0;

    // 初始化等待结构
    if (sock_wait_init(&tcp->snd.wait) < 0) {
        dbg_error(DBG_TCP, "create snd.wait failed");
        goto alloc_failed;
    }
    tcp->base.snd_wait = &tcp->snd.wait;

    if (sock_wait_init(&tcp->rcv.wait) < 0) {
        dbg_error(DBG_TCP, "create rcv.wait failed");
        goto alloc_failed;
    }
    tcp->base.rcv_wait = &tcp->rcv.wait;

    if (sock_wait_init(&tcp->conn.wait) < 0) {
        dbg_error(DBG_TCP, "create conn.wait failed");
        goto alloc_failed;
    }
    tcp->base.conn_wait = &tcp->conn.wait;
    return tcp;
alloc_failed:
    if (tcp->base.snd_wait) {
        sock_wait_destroy(tcp->base.snd_wait);
    }
    if (tcp->base.rcv_wait) {
        sock_wait_destroy(tcp->base.rcv_wait);
    }
    if (tcp->base.conn_wait) {
        sock_wait_destroy(tcp->base.conn_wait);
    }
    mblock_free(&tcp_mblock, tcp);
    return (tcp_t *)0;
}

/**
 * @brief 释放一个控制块
 * 只允许最终被TIME-WAIT状态下调用，或者由用户上层间接调用
 */
void tcp_free(tcp_t* tcp) {
    dbg_assert(tcp->state != TCP_STATE_FREE, "tcp free");

    // 在create中，即便没有创建，但由于全部清空，所以下面处理也没问题
    sock_wait_destroy(&tcp->conn.wait);
    sock_wait_destroy(&tcp->snd.wait);
    sock_wait_destroy(&tcp->rcv.wait);

    // 释放回原来的缓存表
    tcp->state = TCP_STATE_FREE;        // 调试用，方便观察
    nlist_remove(&tcp_list, &tcp->base.node);
    mblock_free(&tcp_mblock, tcp);
}

/**
 * @brief 插入TCP块
 */
void tcp_insert (tcp_t * tcp) {
    nlist_insert_last(&tcp_list, &tcp->base.node);

    dbg_assert(tcp_list.count <= TCP_MAX_NR, "tcp free");
}

/**
 * @brief 获取TCP初始序列号，用于新创建一个连接时
 * 根据RFC793：初始序号列根据时钟产生，每4微秒增长一次
 * 这个实现比较麻烦，所以不遵循这个规范，简单的实现
 */
static uint32_t tcp_get_iss(void) {
    static uint32_t seq = 0;

    // 每次调用增长一下，保证不一样就可以了
#if 0
    seq += seq == 0 ? clock() : 305;
#else
    seq += seq == 0 ? 32435 : 305;
#endif
    return seq;
}

/**
 * @brief 中止TCP连接，进入CLOSED状态，通知应用程序
 * 释放资源的工具由应用调用close完成
 */
net_err_t tcp_abort (tcp_t * tcp, int err) {
    tcp_set_state(tcp, TCP_STATE_CLOSED);
    sock_wakeup(&tcp->base, SOCK_WAIT_ALL, err);
    return NET_ERR_OK;
}

/**
 * @brief 为tcp连接做好准备，初始化其中的一些字段值
 */
static net_err_t tcp_init_connect(tcp_t * tcp) {
    // 发送部分初始化
    tcp_buf_init(&tcp->snd.buf, tcp->snd.data, TCP_SBUF_SIZE);
    tcp->snd.iss = tcp_get_iss();
    tcp->snd.una = tcp->snd.nxt = tcp->snd.iss;

    // 接收部分初始化
    tcp->rcv.nxt = 0;

    return NET_ERR_OK;
}

/**
 * @brief 开始TCP连接请求，这里会向远端发送SYN请求进行连接的建立
 */
net_err_t tcp_connect(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len) {
    tcp_t * tcp = (tcp_t *)sock;

    // 只有处于close状态的才能建立连接,其它状态不可以
    if (tcp->state != TCP_STATE_CLOSED) {
        dbg_error(DBG_TCP, "tcp is not closed. connect is not allowed");
        return NET_ERR_STATE;
    }

    // 设置远程IP和端口号
    const struct x_sockaddr_in* addr_in = (const struct x_sockaddr_in*)addr;
    ipaddr_from_buf(&sock->remote_ip, (uint8_t *)&addr_in->sin_addr.s_addr);
    sock->remote_port = x_ntohs(addr_in->sin_port);

    // 检查本地端口号，如果为0，则分配端口号
    if (sock->local_port == NET_PORT_EMPTY) {
        int port = tcp_alloc_port();
        if (port == -1) {
            dbg_error(DBG_TCP, "alloc port failed.");
            return NET_ERR_NONE;
        }
        sock->local_port = port;
    }

    // 检查本地IP地址：为空，根据路由表选择合适的接口IP
    if (ipaddr_is_any(&sock->local_ip)) {
        // 检查路径，看看是否能够到达目的地。不能达到返回错误
        rentry_t * rt = rt_find(&sock->remote_ip);
        if (rt == (rentry_t*)0) {
            dbg_error(DBG_TCP, "no route to dest");
            return NET_ERR_UNREACH;
        }
        ipaddr_copy(&sock->local_ip, &rt->netif->ipaddr);
    }

    // 初始化tcp连接
    net_err_t err;
    if ((err = tcp_init_connect(tcp)) < 0) {
        dbg_error(DBG_TCP, "init conn failed.");
        return err;
    }

    if ((err = tcp_send_syn(tcp)) < 0) {
        dbg_error(DBG_TCP, "send syn failed.");
        return err;
    }
    // 发送SYNC报文，进入sync_send状态
    tcp_set_state(tcp, TCP_STATE_SYN_SENT);

    // 继续等待后续连接的建立
    return NET_ERR_NEED_WAIT;
}


/**
 * @brief 关闭TCP连接
 * 关闭TCP连接，并且回收所TCP资源
 * TODO: tcp的释放
 */
net_err_t tcp_close(sock_t* sock) {
    tcp_t* tcp = (tcp_t*)sock;

    dbg_info(DBG_TCP, "closing tcp: state = %s", tcp_state_name(tcp->state));

    // 针对每一种TCP状态，做不同的关闭处理
    switch (tcp->state) {
        case TCP_STATE_CLOSED:
            dbg_info(DBG_TCP, "tcp already closed");
            tcp_free(tcp);
            return NET_ERR_OK;
        case TCP_STATE_SYN_RECVD:
        case TCP_STATE_SYN_SENT:        // 连接未建立，直接删除
            tcp_abort(tcp, NET_ERR_CLOSE);
            tcp_free(tcp);
            return NET_ERR_OK;
        case TCP_STATE_CLOSE_WAIT:
            // 发送fin通知自己要关闭发送。但要注意，这里并不一定立即发送FIN, 并且要等
            tcp_send_fin(tcp);
            tcp_set_state(tcp, TCP_STATE_LAST_ACK);
            return NET_ERR_NEED_WAIT;
        case TCP_STATE_ESTABLISHED:
            // 发送fin通知自己要关闭发送。但要注意，这里并不一定立即发送FIN
            tcp_send_fin(tcp);
            tcp_set_state(tcp, TCP_STATE_FIN_WAIT_1);
            return NET_ERR_NEED_WAIT;
        default:
            // 这些状态下已经处于主动关闭或已经完毕
            dbg_error(DBG_TCP, "tcp state error[%s]: send is not allowed", tcp_state_name(tcp->state));
            return NET_ERR_STATE;
    }
}

/**
 * @brief 创建一个TCP sock结构
 */
sock_t* tcp_create (int family, int protocol) {
    // 分配一个TCP块
    tcp_t* tcp = tcp_alloc(1, family, protocol);
    if (!tcp) {
        dbg_error(DBG_TCP, "alloc tcp failed.");
        return (sock_t *)0;
    }

    // 插入全局队列中, 注意此时端口号和IP地址全为0，因此不匹配任何不进行任何通信
    tcp_insert(tcp);
    return (sock_t *)tcp;
}

/**
 * TCP模块初始化
 */
net_err_t tcp_init(void) {
    dbg_info(DBG_TCP, "tcp init.");

    mblock_init(&tcp_mblock, tcp_tbl, sizeof(tcp_t), TCP_MAX_NR, NLOCKER_NONE);
    nlist_init(&tcp_list);

    dbg_info(DBG_TCP, "init done.");
    return NET_ERR_OK;
}
