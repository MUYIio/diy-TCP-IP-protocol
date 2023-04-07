/**
 * @file dns.c
 * @author lishutong(527676163@qq.com)
 * @brief DNS客户端协议实现
 * @version 0.1
 * @date 2022-12-02
 * 
 * @copyright Copyright (c) 2022
 * 目前是针对每一个域名做一次查询，这样简单一些。虽然也可以一次查询多个，但是这样处理起来比较麻烦。
 */
#include "netapi.h"
#include "dns.h"
#include "timer.h"
#include "udp.h"
#include "fixq.h"
#include "mblock.h"
#include "dbg.h"
#include "tools.h"

static dns_entry_t dns_entry_tbl[DNS_ENTRY_SIZE];       // DNS缓存表
static dns_server_t dns_server_tbl[DNS_SERVER_MAX];     // DNS服务器列表
static net_timer_t entry_update_timer;
static udp_t * dns_udp;
static uint8_t working_buf[DNS_WORKING_BUF_SIZE];
static uint16_t id;
static nlist_t req_list;                                 // 请求列表
static mblock_t req_mblock;                       // 请求分配结构
static dns_req_t dns_req_list[DNS_REQ_SIZE];

#define IPV4_FMT        "%d.%d.%d.%d"
#define dbg_ipv4(ipaddr)    ipaddr.a_addr[0], ipaddr.a_addr[1], ipaddr.a_addr[2], ipaddr.a_addr[3]

#if DBG_DISP_ENABLED(DBG_DNS)

static void show_entry_list (void) {
    int idx = 0;

    plat_printf("----------------dns entry list----------\n");
    for (int i = 0; i < DNS_ENTRY_SIZE; i++) {
        dns_entry_t * entry = dns_entry_tbl + i;
        if (ipaddr_is_any(&entry->ipaddr)) {
            continue;
        }

        plat_printf("%d: %s ttl(%d) age(%d) "IPV4_FMT"\n", idx++,
                entry->domain_name, entry->ttl, entry->age, dbg_ipv4(entry->ipaddr));
    }
    plat_printf("--------------------------------\n\n");
}

static void show_server_list (void) {
    int idx = 0;

    plat_printf("----------------dns server list----------\n");
    for (int i = 0; i < DNS_SERVER_MAX; i++) {
        dns_server_t * server = dns_server_tbl + i;
        if (ipaddr_is_any(&server->ipaddr)) {
            continue;
        }

        plat_printf("%d: "IPV4_FMT"\n", idx++, dbg_ipv4(server->ipaddr));
    }
    plat_printf("--------------------------------\n\n");
}

static void show_req_list (void) {
    int idx = 0;

    plat_printf("----------------dns req list----------\n");
    nlist_node_t * node;
    nlist_for_each(node, &req_list) {
        dns_req_t * req = nlist_entry(node, dns_req_t, node);
        
        plat_printf("%d: name(%s) query(%d) server:"IPV4_FMT" ip:"IPV4_FMT
                    " retry tmo: %d, retry_cnt: %d\n", 
                    idx++, 
                    req->domain_name,
                    req->query_id, 
                    dbg_ipv4(dns_server_tbl[req->server].ipaddr),
                    dbg_ipv4(req->ipaddr),
                    req->retry_tmo, req->retry_cnt);
    }
    plat_printf("--------------------------------\n\n");
}
#else
#define show_entry_list() 
#define show_server_list()
#define show_req_list()
#endif

/**
 * @brief 跳问题或回答的名称字段
 * 如果中间出错，返回0.
 * 要特别注意返回值的问题，稍微出错会导致后续包解析出问题
 */
static const uint8_t * domain_name_skip(const uint8_t * name, size_t size) {
    const uint8_t * c = name;
    const uint8_t * end = name + size;
    while (*c && (c < end)) {
        // 压缩标签，2个字节；非压缩，取计数累加
        if ((*c & 0xc0) == 0xc0) {
            // 一个域名仅能包含一个指针，要么只有两个字节就只包含一个指针，要么只在结尾部分跟随一个指针
            // 压缩标签无需以'\0'结束
            c += 2;
            goto skip_end;
        } else {
            c += *c;
        }
    }

    // 非压缩标签没有'\0'，这里针对普通标签判断，跳过'\0'
    if (*c == '\0') {
        c++;
    }
skip_end:
    // 跳过字符符结束符'\0'
    return c >= end ? (const uint8_t *)0 : c;
}

/**
 * @brief 比较域名与问题、回答中的名称
 * 要特别注意返回值的问题，稍微出错会导致后续包解析出问题
 */
static const char * domain_name_cmp(const char * domain_name, const char * name, size_t size) {
    const char * src = domain_name;
    const char * dest = name;

    // 这里不处理压缩标准，因为压缩标签指向的是前面的某个地方
    // 目前我们只检查问题域，这里不存在压缩标签
    while (*src) {
        int cnt = *dest++;
        for (int i = 0; i < cnt; i++) {
            // 不相同则退出
            if (*dest++ != *src++) {
                return (const char *)0;
            }
        }

        // 到这里, c指向了.，dest指向了数字值或者已经结束
        if (*src == '\0') {
            break;
        } else if ((*src++ != '.')) {
            return (const char *)0;
        }
    }
    return (dest >= (name + size)) ? (const char *)0 : dest + 1; // 跳过'\0'
}

/**
 * @brief 添加问题区段
 */
static uint8_t * add_query_field (const char * domain_name, char * buf, size_t size) {
    // 检查长度大小：包含字符串有效长，开头的.和结束的'\0'
    if (size < (sizeof(dns_qfield_t) + plat_strlen(domain_name) + 2)) {
        dbg_error(DBG_DNS, "no enough space for query: %s", domain_name);
        return (uint8_t *)0;
    }

    // 写入名字区域。先写入整个字符，构造成多个以.+字符串的形式
    char * name_buf = buf;
    name_buf[0] = '.'; 
    plat_strcpy(name_buf + 1, domain_name);
    
    // 然后将所有的.换成其之后的字符串长度
    char * c = name_buf;
    while (*c) { 
        if (*c == '.') {
            // 统计后续字符串长度
            char * dot = c++;
            while (*c && (*c != '.')) {
                c++;
            }
            *dot = (uint8_t)(c - dot - 1);
        } else {
            c++;
        }
    }
    *c++ = '\0';

    dns_qfield_t * f = (dns_qfield_t *)c;
    f->class = htons(DNS_QUERY_CLASS_INET);
    f->type = htons(DNS_QUERY_TYPE_A);
    return (uint8_t *)f + sizeof(dns_qfield_t);
}

/**
 * @brief 针对指定的entry表，发送查询报文
 */
static net_err_t dns_send_query (dns_req_t * req) {
    dbg_info(DBG_DNS, "send query %s to "IPV4_FMT"\n", req->domain_name, dbg_ipv4(dns_server_tbl[req->server].ipaddr));

    // 构造DNS查询包头
    dns_hdr_t * dns_hdr = (dns_hdr_t *)working_buf;
    dns_hdr->id = htons(req->query_id);
    dns_hdr->flags.all = 0;
    dns_hdr->flags.qr = 0;          // 查询消息，可不用写
    dns_hdr->flags.tc = 1;          // 只返回前512字节字符
    dns_hdr->flags.rd = 1;          // 期望递归
    dns_hdr->flags.all = htons(dns_hdr->flags.all);
    dns_hdr->qdcount = htons(1);    // 暂时每次只查一个吧
    dns_hdr->ancount = 0;
    dns_hdr->nscount = 0;
    dns_hdr->arcount = 0;

    // 填充1个问题区段
    uint8_t * buf = working_buf + sizeof(dns_hdr_t);
    buf = add_query_field(req->domain_name, (char *)buf, sizeof(working_buf) - (buf - working_buf));
    if (!buf) {
        dbg_error(DBG_DNS, "add query question failed.");
        return NET_ERR_MEM;
    }

    // 向网络上发送查询消息
    dns_server_t * server = dns_server_tbl + req->server;
    struct x_sockaddr_in dest;
    plat_memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(DNS_PORT_DEFAULT);
    dest.sin_addr.s_addr = server->ipaddr.q_addr;

    req->query_id = id;       // 注意纪录一下
    return udp_sendto((sock_t *)dns_udp, working_buf, buf - working_buf, 0, 
                            (const struct x_sockaddr *)&dest, sizeof(dest), (ssize_t *)0);
}

/**
 * @brief 插入DNS服务器
 */
dns_server_t * dns_insert_server (const char * ipstr) {
    dns_server_t * server = (dns_server_t *)0;

    ipaddr_t ipaddr;
    if (ipaddr_from_str(&ipaddr, ipstr) < 0) {
        dbg_error(DBG_DNS, "server ip incorrect: %s", ipstr);
        return (dns_server_t *)0;
    }

    // 从表头开始找空闲项，找到后使用该项进行插入和删除
    // 当然别忘了检查是否有重复的项
    for (int i = 0; i < DNS_SERVER_MAX; i++) {
        dns_server_t * curr = dns_server_tbl + i;

        if ((curr->state == DNS_SERVER_FREE)) {
            if (!server) {
                server = curr;
            }
        } else if (ipaddr_is_equal(&curr->ipaddr, &ipaddr)) {
            return curr;
        }
    }

    // 空闲项，更新ip地址
    if (server) {
        ipaddr_copy(&server->ipaddr, &ipaddr);
        server->state = DNS_SERVER_OK;
    }

    show_server_list();
    return server;
}

/**
 * @brief 缺省的DNS服务器
 */
dns_server_t * dns_default_server (void) {
    return dns_server_tbl + 0;
}

/**
 * @brief 获取下一个有效的服务器
 */
int dns_server_next (int curr_idx) {
    while (++curr_idx < DNS_SERVER_MAX) {
        dns_server_t * s = dns_server_tbl + curr_idx;
        if (s->state == DNS_SERVER_OK) {
            return curr_idx;
        }
    }    

    return -1;
}

/**
 * @brief 设置缺省的DNS服务器
 * 这个服务器可能存在也可能不存在。当不存在时，则插入之，如果存在时，则将其更新到第0个表项
 */
net_err_t dns_set_default_server (const char * ipstr) {
    dns_server_t * default_server = dns_server_tbl + 0;

    // 已经是缺省的服务器，退出
    ipaddr_t ipaddr;
    if (!ipaddr_from_str(&ipaddr, ipstr) && ipaddr_is_equal(&ipaddr, &default_server->ipaddr)) {
        return NET_ERR_OK;
    }

    // 如果不是缺省的，则要考虑插入或者删除。先遍历整个列表，看看是否有已经存在的。
    // 同时在这个过程中，纪录空闲的表项
    // 地址不同，则遍历整个表，找同名的或者空闲的
    dns_server_t temp;
    dns_server_t * free = (dns_server_t *)0;
    int free_idx, exist = 0;
    for (int i = 0; i < DNS_SERVER_MAX; i++) {
        dns_server_t * server = dns_server_tbl + i;
        if (server->state == DNS_SERVER_FREE) {
            if (!free) {
                free = server;
                free_idx = i;
            }
            continue;
        }

        // 已经存在，暂存其内容
        if (ipaddr_is_equal(&ipaddr, &server->ipaddr)) {
            exist = 1;
            plat_memcpy(&temp, server, sizeof(dns_server_t));
            break;
        }
    }

    // 没有找到已经存在的或者空闲的。则没有办法插入或者调整，退出
    if (!exist && !free) {
        dbg_error(DBG_DNS, "server queue full. set default server failed.");
        return NET_ERR_FULL;
    } else {
        // 有同名或者空闲的项，均需将将当前表项前面的所有项往右移
        while (free_idx > 0) {
            dns_server_t * from = dns_server_tbl + free_idx - 1;
            dns_server_t * to = from + 1;
            plat_memcpy(to, from, sizeof(dns_server_t));
            free_idx--;
        }

        if (exist) {
            plat_memcpy(dns_server_tbl + 0, &temp, sizeof(dns_server_t));
        } else {
            ipaddr_copy(&dns_server_tbl[0].ipaddr, &ipaddr);
            dns_server_tbl[0].state = DNS_SERVER_OK;
            dbg_dump_ip(DBG_DNS, "set new dns server: %s", &ipaddr);
        }

        show_server_list();
        return NET_ERR_OK;
    }
}

/**
 * @brief 在DNS缓存表中查找指定域名对应的表项
 * 查找出来的可能是匹配的项，也可能是最老的项
 */
dns_entry_t * dns_entry_find (const char * domain_name) {
    // 遍历列表，找稳定项，且名称相同的项
    for (int i = 0; i < DNS_ENTRY_SIZE; i++) {
        dns_entry_t * curr = dns_entry_tbl + i;
        if (!ipaddr_is_any(&curr->ipaddr)) {
            // 忽略大小写比较字符串名称，要求名称必须完全匹配
            if ((plat_stricmp(domain_name, curr->domain_name) == 0)) {
                dbg_info(DBG_DNS, "found dns entry: %s %s", curr->domain_name, domain_name);
                return NET_ERR_OK;
            }   
        }
    }

    return (dns_entry_t *)0;
}

/**
 * @brief 初始化DNS表项
 */
static void dns_entry_init (dns_entry_t * entry, const char * domain_name, int ttl, ipaddr_t * ipaddr) {
    entry->age = 0;
    entry->ttl = ttl;
    ipaddr_copy(&entry->ipaddr, ipaddr);
    plat_strncpy(entry->domain_name, domain_name, DNS_DOMAIN_NAME_MAX - 1);
    entry->domain_name[DNS_DOMAIN_NAME_MAX - 1] = '\0';
}

/**
 * @brief 释放DNS表项
 */
static void dns_entry_free (dns_entry_t * entry) {
    ipaddr_set_any(&entry->ipaddr);
}

/**
 * @brief 插入一个新的DNS表项
 * 如果有空闲的，则使用空闲的，否则将重要最老的项
 */
static void dns_entry_insert (const char * domain_name, int ttl, ipaddr_t * ipaddr) {
    dns_entry_t * oldest = (dns_entry_t *)0;
    dns_entry_t * next = (dns_entry_t *)0;;

    int age = -1;
    for (int i = 0; i < DNS_ENTRY_SIZE; i++) {
        dns_entry_t * entry = dns_entry_tbl + i;

        // 找到空闲项后立即退出或者已经存在的
        if (ipaddr_is_any(&entry->ipaddr) || (plat_strcmp(domain_name, entry->domain_name) == 0)) {
            next = entry;
            break;
        }

        // 纪录最老的
        if (entry->age > age) {
            oldest = entry;
            age = entry->age;
        }
    }

    // 优先使用空闲的
    next = next ? next : oldest;
    dns_entry_init(next, domain_name, ttl, ipaddr);

    // 调试信息输出
    show_entry_list();
}

/**
 * @brief 申请一个查询请求结构
 */
dns_req_t * dns_alloc_req (void) {
    return mblock_alloc(&req_mblock, 0);
}

void dns_free_req (dns_req_t * req) {
    mblock_free(&req_mblock, req);
}

/**
 * @brief 加入查询队列
 */
static void dns_req_add (dns_req_t * req) {
    req->server = 0;                // 缺省从第0个查起
    req->query_id = ++id;      // 纪录一下这个ID值以结构中
    req->err = NET_ERR_OK;
    req->retry_cnt = 0;
    req->retry_tmo = 0;
    ipaddr_set_any(&req->ipaddr);
    nlist_insert_last(&req_list, &req->node);

    show_req_list();
}

/**
 * @brief 移除请求项
 */
static void dns_req_remove (dns_req_t * req,  net_err_t err) {
    // 没有服务器可以重试了，删除该请求
    nlist_remove(&req_list, &req->node);

    // 刷新请求结构
    req->err = err;
    if (err < 0) {
        ipaddr_set_any(&req->ipaddr);
    }

    if (req->wait_sem != SYS_SEM_INVALID) {
        sys_sem_notify(req->wait_sem);
        req->wait_sem = SYS_SEM_INVALID;
    }

    show_req_list();
}

/**
 * @brief 当请求出现错误时的处理
 */
static void dns_req_fail(dns_req_t * req, net_err_t err, int stop_query) {
    // 当无需停止，且未超过最大重试次数时，重试发送
    if (!stop_query && (++req->retry_cnt < DNS_QUERY_RETRY_CNT)) {
        // 当前服务器的查询已经超时，换一个服务器试下。当没有时，则释放掉
        req->server = dns_server_next(req->server);
        if (req->server >= 0) {
            req->retry_tmo = 0;         // 重置超时
            dns_send_query(req);
            return;
        }
    }

    // 需要停止，或者超过最大重试次数，中止
    dns_req_remove(req, NET_ERR_TMO);
}

/**
 * @brief DNS查询请求消息的处理
 * 所有未被立即解析的域名请求，将被暂时插入到就绪列表中处理
 */
net_err_t dns_req_in(func_msg_t* msg) {
    dns_req_t * dns_req = (dns_req_t *)msg->param;

    // 必要的参数检查，地址要求不能超过整体的名称长度
    size_t name_len = plat_strlen(dns_req->domain_name);
    if (name_len >= DNS_DOMAIN_NAME_MAX) {
        dbg_error(DBG_DNS, "domain name too long: %d > %d", name_len, DNS_DOMAIN_NAME_MAX);
        return NET_ERR_PARAM;
    }

    // 检查是否是本机地址，直接返回，这样不用占用表项
    if (plat_strcmp(dns_req->domain_name, "localhost") == 0) {
        ipaddr_set_loop(&dns_req->ipaddr);
        dns_req->err = NET_ERR_OK;
        return dns_req->err;
    }

    // 已经是IP地址，直接处理
    ipaddr_t ipaddr;
    if (ipaddr_from_str(&ipaddr, dns_req->domain_name) == NET_ERR_OK) {
        ipaddr_copy(&dns_req->ipaddr, &ipaddr);
        dns_req->err = NET_ERR_OK;
        return dns_req->err;
    }

    // 如果已经存在，找到后复制后直接返回
    dns_entry_t * entry = dns_entry_find(dns_req->domain_name);
    if (entry) {
        // 找到已解析的项，直接返回
        ipaddr_copy(&dns_req->ipaddr, &entry->ipaddr);
        dns_req->err = NET_ERR_OK;
        return dns_req->err;
    }

    // 任务需要进入等待状态，等待查询结果
    dns_req->wait_sem = sys_sem_create(0);
    if (dns_req->wait_sem == SYS_SEM_INVALID) {
        dbg_error(DBG_DNS, "create sem failed.");
        return NET_ERR_SYS;
    }

    // 插入请求队列中，并发送请求
    dns_req_add(dns_req);
    net_err_t err = dns_send_query(dns_req);
    if (err < 0) {
        dbg_error(DBG_DNS, "send dns query failed. err=%d", err);
        goto dns_req_end;
    }

    return NET_ERR_OK;
dns_req_end:
    if (dns_req->wait_sem != SYS_SEM_INVALID) {
        sys_sem_free(dns_req->wait_sem);
        dns_req->wait_sem = SYS_SEM_INVALID;
    }
    return err;
}

/**
 * @brief DNS表更新定时处理
 * 定时器会扫描整个表，并做不同的处理
 * 对于正在请求的表项，则会适当进行重新请求，包括简单重试或者换服务器
 * 对于已经得到结果的表项，如果生存时间过多，会进行删除
 */
static void dns_update_tmo (struct _net_timer_t* timer, void * arg) {
    // 增加age计数，有没有可能回绕呢？暂时不考虑
    for (int i = 0; i < DNS_ENTRY_SIZE; i++) {
        dns_entry_t * entry = dns_entry_tbl + i;
        if (ipaddr_is_any(&entry->ipaddr)) {
            continue;
        }

        // 稳定的项，到达生成时间后，就删除之。不自动查询了，让应用层自己去查
        // 因为应用获得IP地址后，比较少再重新使用DNS查询
        if (!entry->ttl || (--entry->ttl == 0)) {
            dns_entry_free(entry);
            show_entry_list();
        }
    }

    // 遍历列表，进行查询超时的处理或者重传超时
    nlist_node_t * curr, * next;
    for (curr = nlist_first(&req_list); curr; curr = next) {
        next = nlist_node_next(curr);

        // 每秒查询一次
        dns_req_t * req = nlist_entry(curr, dns_req_t, node);
        dns_send_query(req);
        show_req_list();

        if (++req->retry_tmo >= DNS_QUERY_RETRY_TMO) {
            // 本次超时，重新查询
            req->retry_tmo = 0;
            
            // 更换服务器继续查询
            dns_req_fail(req, NET_ERR_TMO, 0);
        }
    }
}

/**
 * @brief 判断是还是否DNS数据包到达
 */
int dns_is_arrive (udp_t * udp) {
    return udp == dns_udp;
}

/**
 * @brief 当接收到 DNS报文时的处理
 */
void dns_in (void) {
    ssize_t rcv_len;
    struct x_sockaddr_in src;
    x_socklen_t addr_len;

    // 读取数据包内容到缓存中
    net_err_t err = udp_recvfrom((sock_t *)dns_udp, working_buf, sizeof(working_buf), 0, (struct x_sockaddr *)&src, &addr_len, &rcv_len);
    if (err < 0) {
        dbg_error(DBG_DNS, "rcv udp error: %d", err);
        return;
    }

    // 大小检查，至少1个问题和一个响应，不考虑域名
    if (rcv_len <= (ssize_t)(sizeof(dns_hdr_t) + sizeof(dns_qfield_t) + sizeof(dns_afield_t))) {
        dbg_warning(DBG_DNS, "dns packet too small. %d", rcv_len);
        return;
    }
    const uint8_t * rcv_start = working_buf;
    const uint8_t * rcv_end = working_buf + rcv_len;

    dns_hdr_t * dns_hdr = (dns_hdr_t *)rcv_start;
    dns_hdr->id = ntohs(dns_hdr->id);
    dns_hdr->flags.all = ntohs(dns_hdr->flags.all);
    dns_hdr->qdcount = ntohs(dns_hdr->qdcount);
    dns_hdr->ancount = ntohs(dns_hdr->ancount);
    dns_hdr->nscount = ntohs(dns_hdr->nscount);
    dns_hdr->arcount = ntohs(dns_hdr->arcount);

    // 遍历请求列表，找到匹配的项
    // 遍历列表，进行查询超时的处理或者重传超时
    nlist_node_t * curr;
    nlist_for_each(curr, &req_list) {
        dns_req_t * req = nlist_entry(curr, dns_req_t, node);
        if (req->query_id != dns_hdr->id) {
            continue;
        }

        // 相同id的项，做一些更为细致地检查
        if (dns_hdr->flags.qr == 0) {
            dbg_warning(DBG_DNS, "not a responsed");
            goto req_failure;
        }

        // 不允许截断的消息
        if (dns_hdr->flags.tc == 1) {
            dbg_warning(DBG_DNS, "response truncated");
            goto req_failure;
        }

        // 服务器应当能递归查询
        if (dns_hdr->flags.ra == 0) {
            dbg_warning(DBG_DNS, "recursion not supported");
            goto req_failure;
        }

        // 检查是否有错误。对于服务器的问题，更换服务器并重试。如果是其它问题，则释放请求结构
        net_err_t err = NET_ERR_OK;
        int stop_query = 0;
        switch (dns_hdr->flags.rcode) {
        // 以下应当更换服务器
        case DNS_ERR_NONE:
            // 没有错误
            break;
        case DNS_ERR_NOTIMP:
            dbg_warning(DBG_DNS, "server reply: not support");
            err = NET_ERR_NOT_SUPPORT;
            goto req_failure;
        case DNS_ERR_REFUSED:
            dbg_warning(DBG_DNS, "server reply: refused");
            err = NET_ERR_REFUSED;
            goto req_failure;
        case DNS_ERR_SERV_FAIL:
            dbg_warning(DBG_DNS, "server reply: server failure");
            err = NET_ERR_SERVER_FAILURE;
            goto req_failure;
        case DNS_ERR_NXMOMAIN:
            dbg_warning(DBG_DNS, "server reply: domain not exist");
            err = NET_ERR_NOT_EXIST;
            goto req_failure;
        // 以下直接删除请求
        case DNS_ERR_FORMAT:
            dbg_warning(DBG_DNS, "server reply: format error");
            err = NET_ERR_FORMAT;
            stop_query = 1;         // 停止查询
            goto req_failure;
        default:
            dbg_warning(DBG_DNS, "unknow error");
            err = NET_ERR_UNKNOW;
            stop_query = 1;         // 停止查询
            goto req_failure;
        }

        // 检查响应答案，至少要1个。有时不只一个，因为可能返回CNAME纪录
        if (dns_hdr->qdcount < 1) {
            dbg_warning(DBG_DNS, "query count != 1");
            err = NET_ERR_FORMAT;
            goto req_failure;
        }
        rcv_start += sizeof(dns_hdr_t);
        rcv_start = (const uint8_t *)domain_name_cmp(req->domain_name, (const char *)rcv_start, rcv_end - rcv_start);
        if (rcv_start == (uint8_t *)0) {
            dbg_warning(DBG_DNS, "domain name not match");
            err = NET_ERR_FORMAT;
            goto req_failure;
        }

        // 检查问题其它字段，首先是大小
        if (rcv_start + sizeof(dns_qfield_t) > rcv_end) {
            dbg_warning(DBG_DNS, "size error");
            err = NET_ERR_SIZE;
            goto req_failure;
        }

        // 必须为互联网类
        dns_qfield_t * qf = (dns_qfield_t *)rcv_start;
        if (qf->class != ntohs(DNS_QUERY_CLASS_INET)) {
            dbg_warning(DBG_DNS, "query class not inet");
            err = NET_ERR_FORMAT;
            goto req_failure;
        }

        // 必须为A纪录类型，暂不支持其它类型
        if (qf->type != ntohs(DNS_QUERY_TYPE_A)) {
            dbg_warning(DBG_DNS, "query class not inet");
            err = NET_ERR_FORMAT;
            goto req_failure;
        }
        rcv_start += sizeof(dns_qfield_t);

        // 检查响应区域
         // 响应区域至少要一个，当然如果有多个我们只读取一个。下面最好也检查一下，是否是只有一个
        if (dns_hdr->ancount < 1) {
            dbg_warning(DBG_DNS, "no answer");
            err = NET_ERR_NO_REPLY;
            goto req_failure;
        }

        // 循环遍历整个列表，找出A纪录项
        for (int i = 0; (i < dns_hdr->ancount) && (rcv_start < rcv_end); i++) {
            // 跳过域名，不做检查
            rcv_start = domain_name_skip(rcv_start, rcv_end - rcv_start);
            if (rcv_start == (uint8_t *)0) {
                dbg_warning(DBG_DNS, "size error");
                err = NET_ERR_FORMAT;
                goto req_failure;
            }

            // 检查其余字段，首先空间要够
            if (rcv_start + sizeof(dns_afield_t) > rcv_end) {
                dbg_warning(DBG_DNS, "size error");
                err = NET_ERR_FORMAT;
                goto req_failure;
            }

            // 进行必要的检查后取结果
            dns_afield_t * af = (dns_afield_t *)rcv_start;
            if ((af->class == ntohs(DNS_QUERY_CLASS_INET)) 
                && (af->type == ntohs(DNS_QUERY_TYPE_A)) 
                && (af->rd_len == ntohs(IPV4_ADDR_SIZE))) {
                // 获取IP地址，同时往缓存表中插入新表项 
                ipaddr_from_buf(&req->ipaddr, (uint8_t *)af->rdata);
                dns_entry_insert(req->domain_name, ntohl(af->ttl), &req->ipaddr);

                dbg_info(DBG_DNS, "recv dns A type: %s %s", req->domain_name);
                dbg_dump_ip(DBG_DNS, "ipaddr:", &req->ipaddr);

                // 给应用发通知，通知解析完毕，退出解析
                dns_req_remove(req, NET_ERR_OK);
                return;
            }

            rcv_start += sizeof(dns_afield_t) + ntohs(af->rd_len) - 2;  // 减去结构体中的2个字节
        }
req_failure:
        // 出错处理，或者没有合适的解析项，选用下一个服务器进行请求。如果没有合适的服务器，请求将被删除
        dns_req_fail(req, err, stop_query);
        return;
    }
}

/**
 * @brief 初始化DNS
 */
void dns_init (void) {
    plat_memset(dns_entry_tbl, 0, sizeof(dns_entry_tbl));
    plat_memset(dns_server_tbl, 0, sizeof(dns_server_tbl));

    // 缺省DNS服务器设置，建立两个，并设置缺省的项
    dns_insert_server(DNS_SERVER_0);
    dns_insert_server(DNS_SERVER_1);
    dns_set_default_server(DNS_SERVER_DEFAULT);

    // 表刷新定时器处理
    net_timer_add(&entry_update_timer, "dns timer", dns_update_tmo, (void *)0, DNS_UPDATE_PERIOID * 1000, NET_TIMER_RELOAD);

    // 套接字初始化
    dns_udp = (udp_t *)udp_create(AF_INET, IPPROTO_UDP);
    dbg_assert(dns_udp != (udp_t *)0, "create udp socket failed");

    // 建立请求表
    mblock_init(&req_mblock, dns_req_list, sizeof(dns_req_t), DNS_REQ_SIZE, NLOCKER_THREAD);
}
