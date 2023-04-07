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
#include "tcp_out.h"
#include "exmsg.h"
#include "tcp_state.h"
#include "socket.h"
#include "timer.h"

tcp_t tcp_tbl[TCP_MAX_NR];           // tcp控制块数组
mblock_t tcp_mblock;                 // 空闲TCP控制块链表
nlist_t tcp_list;                     // 已创建的控制块链表

#if DBG_DISP_ENABLED(DBG_TCP)
/**
 * @brief 显示TCP的状态
 */
void tcp_show_info (char * msg, tcp_t * tcp) {
    plat_printf("%s: %s\n", msg, (tcp->state < TCP_STATE_MAX) ? tcp_state_name(tcp->state) : "UNKNOWN");
    plat_printf("    local port: %u, remote port: %u\n", tcp->base.local_port, tcp->base.remote_port);
    plat_printf("    snd.una: %u, snd.nxt: %u, snd.win: %u\n", tcp->snd.una, tcp->snd.nxt, tcp->snd.win);
    plat_printf("    rcv.nxt: %u, rcv.win: %u\n", tcp->rcv.nxt, tcp_rcv_window(tcp));
    plat_printf("    ostate: %s, state: %s, rto: %d\n", tcp_ostate_name(tcp), tcp_state_name(tcp->state), tcp->snd.rto);
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
    char idbuf[10];
    int i = 0;

    plat_printf("-------- tcp list -----\n");

    nlist_node_t * node;
    nlist_for_each(node, &tcp_list) {
        tcp_t * tcp = (tcp_t *)nlist_entry(node, sock_t, node);

        plat_memset(idbuf, 0, sizeof(idbuf));
        plat_printf(idbuf, "%d:", i++);
        tcp_show_info(idbuf, tcp);
    }
}
#endif

/**
 * @brief 分配一个有效的TCP端口
 * 下面的算法中，端口号的选择从1024开始递增，但实际不同系统的端口分配看起来是有一定随意性的
 * 也许可以加一点点随机的处理？
 */
int tcp_alloc_port(void) {
#if 0 // NET_DBG
    // 调用试用，以便每次启动调试时使用的端口都不同，wireshark中显示更干净
    // 并且不会与上次的连接重复，避免对本次连接造成干扰。并不严格的处理，只提供随机处理
    srand((unsigned int)time(NULL));
    int search_idx = rand() % 1000 + NET_PORT_DYN_START;
#else
    static int search_idx = NET_PORT_DYN_START;  // 搜索起点
#endif
    int find_port = -1;

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
    return find_port;
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
        if (ipaddr_is_equal(&s->local_ip, local_ip) && (s->local_port == local_port) &&
            ipaddr_is_equal(&s->remote_ip, remote_ip) && (s->remote_port == remote_port)) {
            return s;
        }

        // 也可能存在不完全匹配的项目，即只匹配了一部分，例如：
        // 对于服务器而言，会有处理listen状态的sock，只包含了本地端口号，其余都未给定
        tcp_t * tcp = (tcp_t *)s;
        if ((tcp->state == TCP_STATE_LISTEN) && (s->local_port == local_port)) {
            // 如果指定了本地IP地址，则IP地址也要相同。否则不检查
            if (!ipaddr_is_any(&s->local_ip) && !ipaddr_is_equal(&s->local_ip, local_ip)) {
                continue;
            } 

            // 端口必须相同。找到后不立即返回，只是记录一下。因为可能后续有完全匹配的项
            if (s->local_port == local_port) {
                match = s;
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
        // 从表中找处于time-wait的结点
        nlist_node_t* node;
        nlist_for_each(node, &tcp_list) {
            tcp_t* s = (tcp_t *)nlist_entry(node, sock_t, node);
            if (s->state == TCP_STATE_TIME_WAIT) {
                // 先销毁掉，再重新分配
                tcp_free(s);
                return (tcp_t*)mblock_alloc(&tcp_mblock, -1);
            }
        }
    }

    return tcp;
}

/**
 * @brief 设置TCP选项
 */
net_err_t tcp_setopt(struct _sock_t* sock,  int level, int optname, const char * optval, int optlen) {
	// 先用osck中的配置项处理
    net_err_t err = sock_setopt(sock, level, optname, optval, optlen);
    if (err == NET_ERR_OK) {
        return NET_ERR_OK;
    } else if ((err < 0) && (err != NET_ERR_UNKNOW)) {
        return err;
    }

    // 必要参数检查
    tcp_t * tcp = (tcp_t *)sock;
    if (level == SOL_SOCKET) {
        if (optlen != sizeof(int)) {
            dbg_error(DBG_TCP, "param size error");
            return NET_ERR_PARAM;
        }
        tcp_keepalive_start(tcp, *(int *)optval);
    } else if (level == SOL_TCP) {
        // TCP选项相关的处理
        switch (optname) {
        case TCP_KEEPIDLE:
            if (optlen != sizeof(int)) {
                dbg_error(DBG_TCP, "param size error");
                return NET_ERR_PARAM;
            }
            tcp->conn.keep_idle = *(int *)optval;
            tcp_keepalive_restart(tcp);
            return NET_ERR_OK;
        case TCP_KEEPINTVL:
            if (optlen != sizeof(int)) {
                dbg_error(DBG_TCP, "param size error");
                return NET_ERR_PARAM;
            }
            tcp->conn.keep_intvl = *(int *)optval;
            tcp_keepalive_restart(tcp);
            return NET_ERR_OK;
        case TCP_KEEPCNT:
            if (optlen != sizeof(int)) {
                dbg_error(DBG_TCP, "param size error");
                return NET_ERR_PARAM;
            }
            tcp->conn.keep_cnt = *(int *)optval;
            tcp_keepalive_restart(tcp);
            return NET_ERR_OK;
        default:
            dbg_error(DBG_TCP, "unknowm param");
            break;
        }
    }
    return NET_ERR_PARAM;
}

/**
 * @brief 分配一个TCP块，该控制块用于用于主动打开或者被动打开
 * 对于处理listen状态的TCP，要避免锁住
 */
static tcp_t* tcp_alloc(int wait, int family, int protocol) {
    static const sock_ops_t tcp_ops = {
        .connect = tcp_connect,
        .close = tcp_close,
        .send = tcp_send,
        .recv = tcp_recv,
        .setopt = tcp_setopt,
        .bind = tcp_bind,
        .listen = tcp_listen,
        .accept = tcp_accept,
        .destroy = tcp_destory,
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
    tcp->parent = (tcp_t *)0;
    tcp->state = TCP_STATE_CLOSED;
    tcp->mss = 0;

    // 连接部分初始化
    tcp->conn.backlog = 0;
    tcp->flags.keep_enable = 0;
    tcp->conn.keep_idle = TCP_KEEPALIVE_TIME;
    tcp->conn.keep_intvl = TCP_KEEPALIVE_INTVL;
    tcp->conn.keep_cnt = TCP_KEEPALIVE_PROBES;

    // 发送部分初始化，缓存不必初始化，在建立连接时再做
    tcp->snd.una = tcp->snd.nxt = tcp->snd.iss - 0;
    tcp->snd.win = 0;
    tcp->snd.ostate = TCP_OSTATE_IDLE;
    tcp->snd.rttseq = 0;
    tcp->snd.srtt = 0;
    tcp->snd.rttvar = 0;
    tcp->snd.rto = TCP_SYN_RTO;
    tcp->snd.rexmit_cnt = 0;
    tcp->snd.rexmit_max = 0;
    tcp->snd.rto = TCP_INIT_RTO;        // 设置缺省的RTO

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
    mblock_free(&tcp_mblock, tcp);
    return (tcp_t *)0;
}

/**
 * @brief 释放一个控制块
 * 只允许最终被TIME-WAIT状态下调用，或者由用户上层间接调用
 */
void tcp_free(tcp_t* tcp) {
    dbg_assert(tcp->state != TCP_STATE_FREE, "tcp free");
    tcp_kill_all_timers(tcp);

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
 * @brief keepalive定时超时
 */
static void tcp_keepalive_tmo(struct _net_timer_t* timer, void * arg) {
    tcp_t * tcp = (tcp_t *)arg;

    // 未超重试次数，继续尝试, 但以更小的时间发送
    if (++tcp->conn.keep_retry <= tcp->conn.keep_cnt) {
        // 发送keepalive报文
        tcp_send_keepalive(tcp);
        net_timer_add(&tcp->conn.timer, "keepalive", tcp_keepalive_tmo, tcp, tcp->conn.keep_intvl*1000, 0);
        dbg_warning(DBG_TCP, "tcp keepalive tmo, retrying: %d", tcp->conn.keep_retry);
    } else {
        // 通知应用，TCP已经关闭
        tcp_send_reset_for_tcp(tcp);
        tcp_abort(tcp, NET_ERR_TMO);
        dbg_error(DBG_TCP, "tcp keepalive tmo, give up");
    }
}

/**
 * @brief 启动tcp keepalive定时器
 */
static void keepalive_start_timer (tcp_t * tcp) {
    // 由关闭到运行
    switch (tcp->state) {
    case TCP_STATE_CLOSED:
    case TCP_STATE_SYN_SENT:
    case TCP_STATE_SYN_RECVD:
    case TCP_STATE_LISTEN:
        // 这些状态，没有建立连接，暂不启动keepalive，在后面建立连接之后再启动
        break;
    case TCP_STATE_ESTABLISHED:
        tcp->conn.keep_retry = 0;
        net_timer_add(&tcp->conn.timer, "keepalive", tcp_keepalive_tmo, tcp, tcp->conn.keep_idle*1000, 0);
        dbg_info(DBG_TCP, "tcp keepalive enabled.");
        break;
    case TCP_STATE_CLOSE_WAIT:
    case TCP_STATE_FIN_WAIT_1:
    case TCP_STATE_FIN_WAIT_2:
    case TCP_STATE_CLOSING:
    case TCP_STATE_TIME_WAIT:
        // 这些状态，要求或已经关闭了，不启用
        break;
    default:
        break;
    }
}

/**
 * @brief 启动TCP的keepavlie
 */
void tcp_keepalive_start (tcp_t * tcp, int run) {
    if (tcp->flags.keep_enable && !run) {
        // 由运行到关闭, 关闭定时器即可
        net_timer_remove(&tcp->conn.timer);
        dbg_info(DBG_TCP, "keepalive disabled");
    } else if (!tcp->flags.keep_enable && run) {
        keepalive_start_timer(tcp);
    }
    tcp->flags.keep_enable = run;
}

/**
 * @brief 重启keepalive处理
 * 如果未启用，则不用管他。如果已经启用了，则重启他
 */
void tcp_keepalive_restart (tcp_t * tcp) {
    if (tcp->flags.keep_enable) {
        // 移除后重启定时器
        net_timer_remove(&tcp->conn.timer);
        keepalive_start_timer(tcp);
    }
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
 * @brief 计算接收窗口的大小
 */
int tcp_rcv_window (tcp_t * tcp) {
    // 
    int window = tcp_buf_free_cnt(&tcp->rcv.buf);
    if ((window >= tcp->mss) || (window >= tcp_buf_size(&tcp->rcv.buf) / 2)) {
        return window;
    }

    return 0;
}

/**
 * @brief 清除TCP所有定时器
 * 定时器有三种：坚持定时器、重传定时
 */
void tcp_kill_all_timers (tcp_t * tcp) {
    net_timer_remove(&tcp->snd.timer);
    net_timer_remove(&tcp->conn.timer);
}

/**
 * @brief 中止TCP连接，进入CLOSED状态，通知应用程序
 * 释放资源的工具由应用调用close完成
 */
net_err_t tcp_abort (tcp_t * tcp, int err) {        
    // 如果应用层应用标记了延时释放，即之前已经主动close了，这次则直接释放掉tcp
    // 否则通知应用程序，当前连接已经关闭
    if (tcp->flags.delayed_free) {
        tcp_free(tcp);
    } else {
        tcp_kill_all_timers(tcp);
        tcp_set_state(tcp, TCP_STATE_CLOSED);
        sock_wakeup(&tcp->base, SOCK_WAIT_ALL, err);
     }
    return NET_ERR_OK;
}

/**
 * @brief 检查是否存在与sock同样的地址对的TCP连接
 * 用于避免反复多次建立同一连接
 */
int tcp_conn_exist(sock_t * sock) {
    // 遍历整个列表，寻找sock地址对完全匹配的项
    nlist_node_t* node;
    nlist_for_each(node, &tcp_list) {
        sock_t* curr = (sock_t*)nlist_entry(node, sock_t, node);

        // 跳过自己
        if (curr == sock) {
            continue;
        }

        // 全匹配即表示已经存在
        if (ipaddr_is_equal(&sock->local_ip, &curr->local_ip)
            && ipaddr_is_equal(&sock->remote_ip, &curr->remote_ip)
            && (sock->local_port == curr->local_port)
            && (sock->remote_port == curr->remote_port)) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief 为tcp连接做好准备，初始化其中的一些字段值
 */
static net_err_t tcp_init_connect(tcp_t * tcp) {
    // 重新计算mss，选项不计算在内
    rentry_t* rt = rt_find(&tcp->base.remote_ip);
    if (rt->netif->mtu == 0) {
        tcp->mss = TCP_DEFAULT_MSS;         // RFC1122, 加上IP和TCP头，总共576字节
    } else if (rt->type != NET_RT_LOCAL_NET) {
        tcp->mss = TCP_DEFAULT_MSS;         // RFC1122, 加上IP和TCP头，总共576字节
    } else {
        // 同一网段，使用MTU减去两个头部
        tcp->mss = rt->netif->mtu - sizeof(ipv4_hdr_t) - sizeof(tcp_hdr_t);
    }

    // 发送部分初始化
    tcp_buf_init(&tcp->snd.buf, tcp->snd.data, TCP_SBUF_SIZE);
    tcp->snd.iss = tcp_get_iss();
    tcp->snd.una = tcp->snd.nxt = tcp->snd.iss;
    tcp->snd.win = tcp->mss;
    tcp->snd.ostate = TCP_OSTATE_IDLE;      // 空闲状态
    tcp->snd.rexmit_max = TCP_SYN_RETRIES;  // SYN重发次数

    // 接收部分初始化
    tcp_buf_init(&tcp->rcv.buf, tcp->rcv.data, TCP_RBUF_SIZE);
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

    // 到这里还要检查下这个连接是否已经存在，因为可能在列表中已经存在地址对的情况
    if (tcp_conn_exist(sock)) {
        dbg_error(DBG_TCP, "conn already exist");
        return NET_ERR_EXIST;
    }

    // 初始化tcp连接
    net_err_t err;
    if ((err = tcp_init_connect(tcp)) < 0) {
        dbg_error(DBG_TCP, "init conn failed.");
        return err;
    }

    // 发送SYNC报文，进入sync_send状态
    tcp_set_state(tcp, TCP_STATE_SYN_SENT);
    if ((err = tcp_send_syn(tcp)) < 0) {
        dbg_error(DBG_TCP, "send syn failed.");
        return err;
    }

    // 继续等待后续连接的建立
    return NET_ERR_NEED_WAIT;
}

/**
 * 绑定本地端口。
 * 主要用于服务器，服务器会监听本地IP和某个端口，以便处理收到的数据包
 * 客户端也可用，用于指定本地连接所用的ip和端口号
 *    同一个Socket只可以将1个端口绑定到1个地址上。 （ok）
 *    即使不同的Socket也不能重复绑定相同的地址和端口。  (ok)
 *    不同的Socket可以将不同的端口绑定到相同的IP地址上。 (ok)
 *    不同的Socket可以将相同的端口绑定到不同的IP地址上。 (ok)
 */
net_err_t tcp_bind(sock_t* sock, const struct x_sockaddr* addr, x_socklen_t len) {
    tcp_t * tcp = (tcp_t *)sock;

    // 只有处于close状态的才能建立连接,其它状态不可以
    if (tcp->state != TCP_STATE_CLOSED) {
        dbg_error(DBG_TCP, "tcp is not closed. connect is not allowed");
        return NET_ERR_STATE;
    }

    // 如果已经绑定了，则不允许再次绑定
    if (sock->local_port != NET_PORT_EMPTY) {
        dbg_error(DBG_UDP, "already binded.");
        return NET_ERR_PARAM;
    }

    // 绑定的端口号不能为0
    const struct x_sockaddr_in* addr_in = (const struct x_sockaddr_in*)addr;
    if (addr_in->sin_port == NET_PORT_EMPTY) {
        dbg_error(DBG_TCP, "port is emptry");
        return NET_ERR_PARAM;
    }

    // 如果IP地址不为空，则应为本地某接口的地址
    ipaddr_t local_ip;
    ipaddr_from_buf(&local_ip, (const uint8_t*)&addr_in->sin_addr);
    if (!ipaddr_is_any(&local_ip)) {
        // 查找路由表，检查是否有该项
        rentry_t* rt = rt_find(&local_ip);
        if (rt == (rentry_t*)0) {
            dbg_error(DBG_TCP, "ipaddr error, no netif has this ip");
            return NET_ERR_ADDR;
        }

        // IP地址必须完全一样
        if (!ipaddr_is_equal(&local_ip, &rt->netif->ipaddr)) {
            dbg_error(DBG_TCP, "ipaddr error");
            return NET_ERR_ADDR;
        }
    }

    // 遍历列表，查找有是否有相同绑定的TCP（不检查已经连接的端口）
    nlist_node_t * node;
    nlist_for_each(node, &tcp_list) {
        sock_t * curr = (sock_t *)nlist_entry(node, sock_t, node);

        // 远端端口不非为空，既已经连接，不检查
        if ((sock == curr) || (curr->remote_port != NET_PORT_EMPTY)) {
            continue;
        }
        
        // 本地地址完全匹配，错误
        if (ipaddr_is_equal(&curr->local_ip, &local_ip) && (curr->local_port == addr_in->sin_port)) {
            dbg_error(DBG_TCP, "ipaddr and port already used");
            return NET_ERR_ADDR;
        }
    }

    // 记录下IP地址和端口号， IP地址可能为空
    ipaddr_copy(&sock->local_ip, &local_ip);
    sock->local_port = x_ntohs(addr_in->sin_port);;
    return NET_ERR_OK;
}

/**
 * @brief 清除父子连接关系
 */
int tcp_clear_parent (tcp_t * tcp) {
    nlist_node_t * node;

    // 遍历，寻找子TCP，断开连接
    nlist_for_each(node, &tcp_list) {
        sock_t * sock = nlist_entry(node, sock_t, node);
        tcp_t * child = (tcp_t *)sock;

        // 清除父子连接关系。在backlog队列中的处于自由状态
        if (child->parent == tcp) {
            child->parent = (tcp_t *)0;
        }
    }
    return NET_ERR_NEED_WAIT;
}

/**
 * TCP删除定时处理，用于自动删除定时器
*/
static void tcp_free_tmo(struct _net_timer_t* timer, void * arg) {
    tcp_t* tcp = (tcp_t*)arg;
    tcp_free(tcp);
}

static void tcp_set_delayed_free (tcp_t * tcp) {
    tcp->flags.delayed_free = 1;
    net_timer_remove(&tcp->conn.timer);
    net_timer_add(&tcp->conn.timer, "tcp free", tcp_free_tmo, tcp, TCP_FREE_DELAYED_TMO, 0);
}

/**
 * @brief 关闭TCP连接
 * 
 * tcp关闭需要通信，因此需要一定时间，并且这个中间可能会通信失败，也有可能进入time-wait状态
 * 然后应用程序不能直接等其整个过程，应用需要立即返回。所以这个释放工作就由交协议栈内部自己处理
 * 在开始关闭后，如果需要延迟释放，则启动一个延迟释放的定时器。当超时后，完成整个释放工作。而当
 * 进入time-wait状态时，该定时器将失败
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
        case TCP_STATE_LISTEN:          // listen没有连接，直接删除
            tcp_clear_parent(tcp);
            tcp_abort(tcp, NET_ERR_CLOSE);
            tcp_free(tcp);
            return NET_ERR_OK;
        case TCP_STATE_SYN_RECVD: 
        case TCP_STATE_SYN_SENT:        // 连接未建立，直接删除
            tcp_abort(tcp, NET_ERR_CLOSE);
            tcp_free(tcp);
            return NET_ERR_OK;
        case TCP_STATE_CLOSE_WAIT:
            // 发送fin通知自己要关闭发送。应用不需要等，自己内部完成释放
            tcp_send_fin(tcp);
            tcp_set_state(tcp, TCP_STATE_LAST_ACK);
            tcp_set_delayed_free(tcp);
            return NET_ERR_OK;
        case TCP_STATE_ESTABLISHED: 
            // 发送fin通知自己要关闭发送。应用不需要等，自己内部完成释放
            tcp_send_fin(tcp);
            tcp_set_state(tcp, TCP_STATE_FIN_WAIT_1);
            tcp_set_delayed_free(tcp);
            return NET_ERR_OK;
        case TCP_STATE_FIN_WAIT_1:
        case TCP_STATE_FIN_WAIT_2: 
        case TCP_STATE_CLOSING:
        case TCP_STATE_LAST_ACK:
        case TCP_STATE_TIME_WAIT:
        default: 
            // 这些状态下已经处于主动关闭或已经完毕
            dbg_error(DBG_TCP, "tcp state error[%s]: send is not allowed", tcp_state_name(tcp->state));
            tcp_free(tcp);
            return NET_ERR_STATE;
    }
}

/**
 * @brief 强制删除tcp
 */
void tcp_destory (struct _sock_t * sock) {
    tcp_t * tcp = (tcp_t *)sock;

    // TIME-WAIT状态下，由定时器自动销毁
    if (tcp->state != TCP_STATE_TIME_WAIT) {
        tcp_free((tcp_t *)sock);
    }
}

/**
 * @brief 执行发送请求
 * 发送请求只在部分状态下有效，即对方未主动关闭的情况下
 */
net_err_t tcp_send (struct _sock_t* sock, const void* buf, size_t len, int flags, ssize_t * result_len) {
    tcp_t* tcp = (tcp_t*)sock;

    switch (tcp->state) {
        case TCP_STATE_CLOSED:
            dbg_error(DBG_TCP, "tcp closed: send is not allowed");
            return NET_ERR_NOT_EXIST;
        case TCP_STATE_FIN_WAIT_1:
        case TCP_STATE_FIN_WAIT_2:
        case TCP_STATE_CLOSING:
        case TCP_STATE_TIME_WAIT:
        case TCP_STATE_LAST_ACK:
            // 以上状态，自己关闭了发送，因此不允许再发送
            dbg_error(DBG_TCP, "tcp closed[%s]: send is not allowed", tcp_state_name(tcp->state));
            return NET_ERR_CLOSE;
        case TCP_STATE_ESTABLISHED:
        case TCP_STATE_CLOSE_WAIT: {
            // 只有这两个状态允许使用send接口发送
            // CLOSE-WAIT下是对方关闭了发送，但我们可以发数据过去让对方接收
            break;
        }
        case TCP_STATE_LISTEN:
        case TCP_STATE_SYN_RECVD:
        case TCP_STATE_SYN_SENT:
        default:
            // 监听状态不允许发数据，连接状态未完全建立时也不允许发数据
            // 这种策略虽不符合RFC793，但影响的是只是自己上层的应用程序
            dbg_error(DBG_TCP, "tcp state error[%s]: send is not allowed", tcp_state_name(tcp->state));
            return NET_ERR_STATE;
    }

    // 将数据写入发送缓存中
    ssize_t size = tcp_write_sndbuf(tcp, (uint8_t *)buf, (int)len);
    if (size <= 0) {
        // 缓存可能已满，返回写入0. 上层应用应当等待
        *result_len = 0;
        return NET_ERR_NEED_WAIT; 
    } else {
        *result_len = size;

        // 通知有发送数据事件发送，并进行处理
        tcp_out_event(tcp, TCP_OEVENT_SEND);
        return NET_ERR_OK;
    }
}

/**
 * @brief 执行读请求
 */
net_err_t tcp_recv (struct _sock_t* s, void* buf, size_t len, int flags, ssize_t * result_len) {
    tcp_t* tcp = (tcp_t*)s;

    switch (tcp->state) {
        case TCP_STATE_CLOSING:   // 同时关闭，收到了对方的主动关闭，不能再收数据了 
        case TCP_STATE_LAST_ACK:  // 对方已经关了，自己主动关了
            // 已经完全关闭、自己关闭了发送，不允许再读取
            dbg_error(DBG_TCP, "tcp state error[%s]: recv is closed");
            return NET_ERR_CLOSE;
        case TCP_STATE_CLOSE_WAIT:    // 虽然收到了对方的FIN，但缓存可能仍有数据可读
        case TCP_STATE_ESTABLISHED:   // 正常连接时可允许通信
        case TCP_STATE_FIN_WAIT_1:    // 主动发起关闭发送
        case TCP_STATE_FIN_WAIT_2:    // 主动关闭送已经完成
            // 这些状态是允许接收数据的
            break;
        case TCP_STATE_CLOSED:
            // 关闭状态不允许读取
            dbg_error(DBG_TCP, "tcp closed: recv is not allowed");
            return NET_ERR_NOT_EXIST;
        case TCP_STATE_LISTEN:
        case TCP_STATE_SYN_RECVD:
        case TCP_STATE_SYN_SENT:
        case TCP_STATE_TIME_WAIT:  // 2MS状态
        default:
            // 监听状态不允许发数据，连接状态未完全建立时也不允许发数据
            // 这种策略虽不符合RFC793，但影响的是只是自己上层的应用程序
            dbg_error(DBG_TCP, "tcp state error[%s]: recv is not allowed", tcp_state_name(tcp->state));
            return NET_ERR_STATE;
    }

    // 此次肯定要读取一些数据的，所以如果之前窗口为0，此时可以告知对方了
    int pre_win = tcp_rcv_window(tcp);

    // 检查缓存中是否有数据，如果有返回给用户；如果没有，则等待
    int cnt = tcp_buf_read_rcv(&tcp->rcv.buf, buf, (int)len);
    if (cnt > 0) {
        // 当由0窗口变成非0窗口时报告一下
        if (!pre_win && tcp_rcv_window(tcp)) {
            tcp_send_win_update(tcp);
        }
        *result_len = cnt;
        return NET_ERR_OK;
    }

    // 缓存中没有数据，告知用户需要等待
    return NET_ERR_NEED_WAIT;
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
 * @brief 设置TCP进入Listen状态
 * 
 * 在此之后，TCP将接收输入的TCP连接并进行处理
 */
net_err_t tcp_listen (struct _sock_t* s, int backlog) {
    tcp_t * tcp = (tcp_t *)s;

    // 必要的参数检查
    if (backlog <= 0) {
        dbg_error(DBG_TCP, "backlog(%d) <= 0", backlog);
        return NET_ERR_PARAM;
    }

    // 只有处于close状态的才能建立连接,其它状态不可以
    if (tcp->state != TCP_STATE_CLOSED) {
        dbg_error(DBG_TCP, "tcp is not closed. listen is not allowed");
        return NET_ERR_STATE;
    }

    // 简单设置一下，不影响以前已经连接量。由于原初始值为0，所以不能接收连接。一旦这里设置后，即可允许连接
    tcp->state = TCP_STATE_LISTEN;
    tcp->conn.backlog = backlog;
    return NET_ERR_OK;
}

/**
 * @brief 获取一个TCP连接
 * 如何表示一个TCP在s的队列里，再建立一个链表吗?
 */
net_err_t tcp_accept (struct _sock_t *s, struct x_sockaddr* addr, x_socklen_t* len, struct _sock_t ** client) {
    nlist_node_t * node;

    nlist_for_each(node, &tcp_list) {
        sock_t * sock = nlist_entry(node, sock_t, node);
        tcp_t * tcp = (tcp_t *)sock;

        // 跳过自己和不属于自己的
        if ((sock == s) || (tcp->parent != (tcp_t *)s)) {
            continue;
        }

        // 属于自己的子child，且处于非激活状态
        if (tcp->flags.inactive) {
            struct x_sockaddr_in * addr_in = (struct x_sockaddr_in *)addr;
            plat_memset(addr_in, 0, sizeof(struct x_sockaddr_in));
            addr_in->sin_family = AF_INET;
            addr_in->sin_port = x_htons(tcp->base.remote_port);
            ipaddr_to_buf(&tcp->base.remote_ip, (uint8_t *)&addr_in->sin_addr.s_addr);
            if (len) {
                *len = sizeof(struct x_sockaddr_in);
            }

            tcp->flags.inactive = 0;       // 清除非活动标志

            // 返回给调用者
            *client = (sock_t *)tcp;
            return NET_ERR_OK;
        }

    }
    return NET_ERR_NEED_WAIT;
}


/**
 * @brief 获取一个TCP连接
 * 如何表示一个TCP在s的队列里，再建立一个链表吗?
 */
int tcp_backlog_count (tcp_t * tcp) {
    int count = 0;
    nlist_node_t * node;

    nlist_for_each(node, &tcp_list) {
        sock_t * sock = nlist_entry(node, sock_t, node);
        tcp_t * child = (tcp_t *)sock;

        // 跳过自己和不属于自己的
        if ((child->parent == tcp) && (child->flags.inactive)) {
            count++;
        }
    }
    return NET_ERR_NEED_WAIT;
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

/**
 * @brief 创建一个TCP的副本
 * 注意，生成的TCP已经处理连接状态，因此相关的状态值应当设置好
 */
tcp_t * tcp_create_child (tcp_t * parent, tcp_seg_t * seg) {
    // 创建一个新的子TCP，只从父中拷贝少量相关信息
    tcp_t * child = (tcp_t *)tcp_alloc(0, parent->base.family, parent->base.protocol);
    if (!child) {
        dbg_error(DBG_TCP, "no child tcp");
        return (tcp_t *)0;
    }

    // 必要的复制
    ipaddr_copy(&child->base.local_ip, &seg->local_ip);
    ipaddr_copy(&child->base.remote_ip, &seg->remote_ip);
    child->base.local_port = seg->hdr->dport;
    child->base.remote_port = seg->hdr->sport;
    child->parent = parent;
    child->flags.irs_valid = 1;                 // 初始已经有效了
    child->flags.inactive = 1;                  // 未被aceept接收，不可用
    child->conn.backlog = 0;

    tcp_init_connect(child);                    // 初始连接，计算mss等
    child->rcv.iss = seg->seq;
    child->rcv.nxt = child->rcv.iss + 1;        // 跳过SYN
    child->snd.win = seg->hdr->win;             // 记录窗口大小
    child->snd.wl1_seq = seg->seq;
    child->snd.wl2_ack = seg->hdr->ack;   // 别忘了这个!!!

    tcp_read_options(child, seg->hdr);          // 读取选项

    // 别忘了队列中
    tcp_insert(child);
    return child;
}