/**
 * @file ipv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "ipv4.h"
#include "dbg.h"
#include "tools.h"
#include "protocol.h"
#include "icmpv4.h"
#include "mblock.h"
#include "timer.h"
#include "raw.h"
#include "udp.h"
#include "tcp_in.h"

static uint16_t packet_id = 0;                  // 每次发包的序号

// ip包分片与重组相关
static ip_frag_t frag_array[IP_FRAGS_MAX_NR]; // 分片控制链表
static mblock_t frag_mblock;                    // 分片控制缓存
static nlist_t frag_list;                        // 等待的分片列表
static net_timer_t frag_timer;                      // 分片定时器
    
// 路由表相关配置
static nlist_t rt_list;                          // 路由表
static mblock_t rt_mblock;                      // 邮箱表分配结构
static rentry_t rt_table[IP_RTABLE_SIZE];      // 路由表

#if DBG_DISP_ENABLED(DBG_IP)
void rt_nlist_display(void) {
    plat_printf("Route table:\n");

    for (int i = 0, idx = 0; i < IP_RTABLE_SIZE; i++) {
        rentry_t* entry = rt_table + i;
        if (entry->netif) {
            plat_printf("%d: ", idx++);
            dbg_dump_ip(DBG_IP, "net:", &entry->net);
            plat_printf("\t");
            dbg_dump_ip(DBG_IP, "mask:", &entry->mask);
            plat_printf("\t");
            dbg_dump_ip(DBG_IP, "next_hop:", &entry->next_hop);
            plat_printf("\t");
            plat_printf("if: %s", entry->netif->name);
            plat_printf("\n");
        }
    }
}
#else
#define rt_nlist_display()
#endif

/**
 * 初始化路径表
 */
void rt_init(void) {
    // 建立路由表及其分配结构
    nlist_init(&rt_list);
    mblock_init(&rt_mblock, rt_table, sizeof(rentry_t), IP_RTABLE_SIZE, NLOCKER_NONE);
}

/**
 * 添加一条路由表项
 */
void rt_add(ipaddr_t * net, ipaddr_t* mask, ipaddr_t* next_hop, netif_t* netif) {
    // 分配表项
    rentry_t* entry = (rentry_t*)mblock_alloc(&rt_mblock, -1);
    if (!entry) {
        dbg_warning(DBG_IP, "alloc rt entry failed.");
        return;
    }

    // 初始化并加入路由表
    ipaddr_copy(&entry->net, net);
    ipaddr_copy(&entry->mask, mask);
    entry->mask_1_cnt = ipaddr_1_cnt(mask);
    ipaddr_copy(&entry->next_hop, next_hop);
    entry->netif = netif;
    nlist_insert_last(&rt_list, &entry->node);

    if (ipaddr_is_equal(next_hop, &netif->ipaddr)) {
        // 定向到本地接口
        entry->type = NET_RT_NETIF;
    } else if (!next_hop) {
        // 发往本地某个主机
        entry->type = NET_RT_LOCAL_NET;
    } else {
        // 非定向到本地接口的
        entry->type = NET_RT_OTHER;
    }
}

/**
 * 删除一条路由
 */
void rt_remove(ipaddr_t * net, ipaddr_t * mask) {
    nlist_node_t * node;

    nlist_for_each(node, &rt_list) { 
        rentry_t* entry = nlist_entry(node, rentry_t, node);
        if (ipaddr_is_equal(&entry->net, net) && ipaddr_is_equal(&entry->mask, mask)) {
            nlist_remove(&rt_list, node);

            // 提示一些信息
            dbg_info(DBG_IP, "remove a route info:");
            dbg_dump_ip(DBG_IP, "net:", net);
            dbg_dump_ip(DBG_IP, "mask:", mask);
            plat_printf("\n");

            rt_nlist_display();
            return;
        }
    }
}

/**
 * 搜索路径表，找到想要的表项
 */
rentry_t* rt_find(ipaddr_t * ip) {
    rentry_t* e = (rentry_t*)0;

    nlist_node_t* node;

    // 遍历整个列表
    nlist_for_each(node, &rt_list) {
        rentry_t* entry = nlist_entry(node, rentry_t, node);

        // ip & mask != entry->net就跳过
        ipaddr_t net = ipaddr_get_net(ip, &entry->mask);
        if (!ipaddr_is_equal(&net, &entry->net)) {
            continue;
        }

        // 找到匹配项时，做以下检查
        // 如果是第一次找到匹配的项，或者掩码中1的数量更多，即更匹配，则使用该项
        if (!e || (e->mask_1_cnt < entry->mask_1_cnt)) {
            e = entry;
        }
    }

    return e;
}

/**
 * @brief 设置IP包头的首部长度
 */
static inline void set_header_size(ipv4_pkt_t* pkt, int size) {
    pkt->hdr.shdr = size / 4;
}

/**
 * @brief 获取总的有效IP负载数据区长度
 */
static inline int get_data_size(ipv4_pkt_t* pkt) {
    return pkt->hdr.total_len - ipv4_hdr_size(pkt);
}

/**
 * @brief 获取分片偏移的起始偏移
 */
static inline uint16_t get_frag_start(ipv4_pkt_t* pkt) {
    return pkt->hdr.offset * 8;;
}

/**
 * @brief 获取分片偏移的结束偏移
 */
static inline uint16_t get_frag_end(ipv4_pkt_t* pkt) {
    return get_frag_start(pkt) + get_data_size(pkt);;
}

/**
 * @brief 对IP包头进行大小端转换
 */
static void iphdr_ntohs(ipv4_pkt_t* pkt) {
    pkt->hdr.total_len = x_ntohs(pkt->hdr.total_len);
    pkt->hdr.id = x_ntohs(pkt->hdr.id);
    pkt->hdr.frag_all = x_ntohs(pkt->hdr.frag_all);
}

/**
 * @brief 将IP包头的格式转换为网络端格式
 */
static void iphdr_htons(ipv4_pkt_t* pkt) {
    pkt->hdr.total_len = x_htons(pkt->hdr.total_len);
    pkt->hdr.id = x_htons(pkt->hdr.id);
    pkt->hdr.frag_all = x_ntohs(pkt->hdr.frag_all);
}

#if DBG_DISP_ENABLED(DBG_IP)
static void display_ip_frags(void) {
    nlist_node_t *f_node, * p_node;
    int f_index = 0, p_index = 0;

    plat_printf("DBG_IP frags:");
    for (f_node = nlist_first(&frag_list); f_node; f_node = nlist_node_next(f_node)) {
        ip_frag_t* frag = nlist_entry(f_node, ip_frag_t, node);
        plat_printf("[%d]:\n", f_index++);
        dump_ip_buf("\tip:", frag->ip.a_addr);
        plat_printf("\tid: %d\n", frag->id);
        plat_printf("\ttmo: %d\n", frag->tmo);
        plat_printf("\tbufs: %d\n", nlist_count(&frag->buf_list));

        // 逐个显示各个分片的区间
        plat_printf("\tbufs:\n");
        nlist_for_each(p_node, &frag->buf_list) {
            pktbuf_t * buf = nlist_entry(p_node, pktbuf_t, node);
            ipv4_pkt_t* pkt = (ipv4_pkt_t *)pktbuf_data(buf);

            plat_printf("\t\tB%d[%d - %d], ", p_index++, get_frag_start(pkt), get_frag_end(pkt) - 1);
        }
        plat_printf("\n");
}
    plat_printf("");
}

/**
 * @brief 打印IP数据包
 */
static void display_ip_packet(ipv4_pkt_t* pkt) {
    ipv4_hdr_t* ip_hdr = (ipv4_hdr_t*)&pkt->hdr;

    plat_printf("--------------- ip ------------------ \n");
    plat_printf("    Version:%d\n", ip_hdr->version);
    plat_printf("    Header len:%d bytes\n", ipv4_hdr_size(pkt));
    plat_printf("    Totoal len: %d bytes\n", ip_hdr->total_len);
    plat_printf("    Id:%d\n", ip_hdr->id);
    plat_printf("    Frag offset: 0x%04x\n", ip_hdr->offset);
    plat_printf("    More frag: %d\n", ip_hdr->more);
    plat_printf("    TTL: %d\n", ip_hdr->ttl);
    plat_printf("    Protocol: %d\n", ip_hdr->protocol);
    plat_printf("    Header checksum: 0x%04x\n", ip_hdr->hdr_checksum);
    dbg_dump_ip_buf(DBG_IP, "    src ip:", ip_hdr->dest_ip);
    plat_printf("\n");
    dbg_dump_ip_buf(DBG_IP, "    dest ip:", ip_hdr->src_ip);
    plat_printf("\n");
    plat_printf("--------------- ip end ------------------ \n");

}

#else
#define display_ip_packet(pkt)
#define display_ip_frags()
#endif


/**
 * @brief 释放所有分片控制块所占有的buf
 */
static void frag_free_buf_list (ip_frag_t * frag) {
    nlist_node_t* node;
    while ((node = nlist_remove_first(&frag->buf_list))) {
        pktbuf_t* buf = nlist_entry(node, pktbuf_t, node);
        pktbuf_free(buf);
    }
}

/**
 * @brief 分配一个分片控制块，如果全部占用，则使用最久未用的
 */
static ip_frag_t * frag_alloc(void) {
    // 先从空闲列表中找
    ip_frag_t * frag = mblock_alloc(&frag_mblock, -1);
    if (!frag) {
        // 没有，则从等待的列表中找最老的重用
        nlist_node_t* node = nlist_remove_last(&frag_list);
        frag = nlist_entry(node, ip_frag_t, node);
        if (frag) {
            // 注释释放里面的缓存包
            frag_free_buf_list(frag);
        }
    }

    return frag;
}

/**
 * @brief 回收分片控制块
 */
static void frag_free (ip_frag_t * frag) {
    // 释放buf
    frag_free_buf_list(frag);

    // 从列表移除
    nlist_remove(&frag_list, &frag->node);

    // 回插到分配控制块中
    mblock_free(&frag_mblock, frag);
}

/**
 * @brief 添加一个分片
 */
static void frag_add (ip_frag_t * frag, ipaddr_t* ip, uint16_t id) {
    // 初始化frag结构
    ipaddr_copy(&frag->ip, ip);
    frag->tmo = IP_FRAG_TMO / IP_FRAG_SCAN_PERIOD;
    frag->id = id;
    nlist_node_init(&frag->node);
    nlist_init(&frag->buf_list);

    // 新创建的放在头部
    nlist_insert_first(&frag_list, &frag->node);
}

/**
 * @brief 遍历查找指定的分片控制块, 查找条件，根据ip地址和id
 */
static ip_frag_t* frag_find(ipaddr_t* ip, uint16_t id) {
    nlist_node_t* curr;

    nlist_for_each(curr, &frag_list) {
        ip_frag_t* frag = nlist_entry(curr, ip_frag_t, node);

        // IP地址和id要相同
        if (ipaddr_is_equal(ip, &frag->ip) && (id == frag->id)) {
            // 移动至最前，方便下一次快速查找
            nlist_remove(&frag_list, curr);
            nlist_insert_first(&frag_list, curr);
            return frag;
        }
    }

    return (ip_frag_t*)0;
}

/**
 * @brief 分片扫描定时器
 * 在扫描过程中，如果发现有分片包到达超时，则释放掉该所有分片
 */
static void frag_tmo(net_timer_t* timer, void * arg) {
    nlist_node_t* curr, * next;
    
    // 遍历列表
    dbg_info(DBG_IP, "scan frag");
    for (curr = nlist_first(&frag_list); curr; curr = next) {
        // 先获取后续结点，以免释放方面的问题
        next = nlist_node_next(curr);
        
        // 超时到达后立即释放
        ip_frag_t * frag = nlist_entry(curr, ip_frag_t, node);
        if (--frag->tmo <= 0) {
            frag_free(frag);
        }
    }
}

/**
 * @brief IP分片有序插入的条件判断函数
 * TODO: 其实下面的可以考虑进行合并，从而释放出pktbuf_t结构出来，从而进行一定程序的优化
 */
static net_err_t frag_insert(ip_frag_t * frag, pktbuf_t * buf, ipv4_pkt_t* pkt) {
    // 不允许插入太多，减少缓存用完的可能性
    if (nlist_count(&frag->buf_list) >= IP_FRAG_MAX_BUF_NR) {
        dbg_warning(DBG_IP, "too many buf on frag. drop it.\n");
        frag_free(frag);
        return NET_ERR_FULL;
    }

    // 和定时器链表一样，按顺序插入.不过顺序是从offset从小到达
    nlist_node_t* node;
    nlist_for_each(node, &frag->buf_list) {
        pktbuf_t* curr_buf = nlist_entry(node, pktbuf_t, node);
        ipv4_pkt_t* curr_pkt = (ipv4_pkt_t*)pktbuf_data(curr_buf);

        // 由于测试条件有限，这种非顺序的到达很难测试
        // 会不会存在某个一包地址区间在分片包的地址范围内呢？也许吧
        // 如果出现合并错误，让上层自己处理。tcp/udp都有一些检查工作
        // IP本来就不对数据传输服务做任何保证
        uint16_t curr_start = get_frag_start(curr_pkt);
        if (get_frag_start(pkt) == curr_start) {
            // 起始重叠，可能是IP包重复了，直接丢弃
            return NET_ERR_EXIST;
        } else if (get_frag_end(pkt) <= curr_start) {
            // 结束地址在当前的包的开始地址之前，插入在当前包之前
            nlist_node_t* pre = nlist_node_pre(node);
            if (pre) {
                nlist_insert_after(&frag->buf_list, pre, &buf->node);
            } else {
                nlist_insert_first(&frag->buf_list, &buf->node);
            }
            return NET_ERR_OK;
        }
    }

    // 找不到合适的位置，插入到队尾
    nlist_insert_last(&frag->buf_list, &buf->node);
    return NET_ERR_OK;
}

/**
 * @brief 判断是否所有的分片包都已经到达
 */
static int frag_is_all_arrived(ip_frag_t* frag) {
    int offset = 0;
    ipv4_pkt_t* pkt = (ipv4_pkt_t*)0;

    // 遍历，检查分片空间的连续性
    nlist_node_t* node;
    nlist_for_each(node, &frag->buf_list) {
        pktbuf_t * buf = nlist_entry(node, pktbuf_t, node);
        
        pkt = (ipv4_pkt_t*)pktbuf_data(buf);

        // 偏移量应当相等
        int curr_offset = get_frag_start(pkt);
        if (curr_offset != offset) {
            return 0;
        }

        offset += get_data_size(pkt);
    }

    // 最后一个必须为more_frag == 0
    return pkt ? !pkt->hdr.more : 0;
}

/**
 * @brief 合并frag中所有的缓存包
 * 在判断所有的分片包到达之后，将缓存的所有包按顺序进行组合，生成一个包
 */
static pktbuf_t * frag_join (ip_frag_t * frag) {
    pktbuf_t * target = (pktbuf_t *)0;

    // 由于列表中已经是有序的，因此只要从头部按顺序依次移除合并即可
    // 在合并过程中，为了安全起见，可以再次检查数据地址范围
    nlist_node_t *node;
    while ((node = nlist_remove_first(&frag->buf_list))) {
        pktbuf_t * curr = nlist_entry(node, pktbuf_t, node);

        // 之前没有缓存，将curr作为头部
        if (!target) {
            target = curr;
            continue;
        }

        // 先移去本分片包的头部，只需要保留第一个分片包的头部即可
        // 原包头并没有移除，因为其中有偏移，被用作判断分片是否达到的依据
        ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(curr);
        net_err_t err = pktbuf_remove_header(curr, ipv4_hdr_size(pkt));
        if (err < 0) {
            dbg_error(DBG_IP,"remove header for failed, err = %d\n", err);
            pktbuf_free(curr);      // 不要忘了这个
            goto free_and_return;
        }

        // 建立前后的连接，拼接在一起, curr在join里面会释放掉
        err = pktbuf_join(target, curr);
        if (err < 0) {
            dbg_error(DBG_IP,"join ip frag failed. err = %d\n", err);
            pktbuf_free(curr);      // 不要忘了这个
            goto free_and_return;
        }
    }

    frag_free(frag);
    return target;

free_and_return:
    // 失败，释放掉所有的缓存及frag控制结构
    if (target) {
        pktbuf_free(target);
    }

    frag_free(frag);
    return (pktbuf_t *)0;
}

/**
 * @brief 非IP包分片处理的输入处理
 */
static net_err_t ip_normal_in(netif_t* netif, pktbuf_t* buf, ipaddr_t* src, ipaddr_t * dest) {
    ipv4_pkt_t* pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    display_ip_packet(pkt);

    // 根据不同协议类型做不同处理， 进行多路径分解
    switch (pkt->hdr.protocol) {
    case NET_PROTOCOL_ICMPv4: {
        net_err_t err = icmpv4_in(src, &netif->ipaddr, buf);
        if (err < 0) {
            dbg_warning(DBG_IP, "icmp in failed.\n");
            return err;
        }
        return NET_ERR_OK;
    }
    case NET_PROTOCOL_UDP: {
        // 提交给UDP处理
        net_err_t err = udp_in(buf, src, dest);
        if (err < 0) {
            dbg_warning(DBG_IP, "udp in error. err = %d\n", err);

            // 发送ICMP不可达信息
            if (err == NET_ERR_UNREACH) {
                iphdr_htons(pkt);       // 注意转换回来
                icmpv4_out_unreach(src, &netif->ipaddr, ICMPv4_UNREACH_PORT, buf);
            }
            return err;
        }
        return NET_ERR_OK;
    }
        case NET_PROTOCOL_TCP: {
            // 移去包头后交由TCP模块处理
            pktbuf_remove_header(buf, ipv4_hdr_size(pkt));
            net_err_t err = tcp_in(buf, src, dest);
            if (err < 0) {
                dbg_warning(DBG_IP, "udp in error. err = %d\n", err);
                return err;
            }
            return NET_ERR_OK;
        }
        default: {
            dbg_warning(DBG_IP, "unknown protocol %d, drop it.\n", pkt->hdr.protocol);

            // 其它没有处理的，交给原始套接字层处理, 原始IP数据报
            net_err_t err = raw_in(buf);
            if (err < 0) {
                dbg_warning(DBG_IP, "raw in error. err = %d\n", err);
            }
            return NET_ERR_UNREACH;
        }
    }
}

/**
 * @brief 处理分片IP包的输入
 */
static net_err_t ip_frag_in (netif_t * netif, pktbuf_t * buf, ipaddr_t* src, ipaddr_t* dest) {
    ipv4_pkt_t * curr = (ipv4_pkt_t *)pktbuf_data(buf);

    // 找到分片控制结构。如果没有，则新创建一个
    ip_frag_t * frag = frag_find(src, curr->hdr.id);
    if (!frag) {
        // frag不可能为0，因空闲没有，就会分配最老的
        frag = frag_alloc();
        frag_add(frag, src, curr->hdr.id);
    }

    // 将ip分片包插入到frag中等待
    net_err_t err = frag_insert(frag, buf, curr);
    if (err < 0) {
        dbg_warning(DBG_IP, "frag insert failed.");
        return err;
    }

    // 注意!!! 下面的代码可能返回错误值，这将导致buf在上层再次释放一次
    // 因此，为避免释放两次，均返回NET_ERR_OK
    // 测试是否能组成一个完整的数据包，可以则合并处理
    if (frag_is_all_arrived(frag)) {
        pktbuf_t * full_buf = frag_join(frag);
        if (!full_buf) {
            dbg_error(DBG_IP,"join all ip frags failed.\n");
            display_ip_frags();
            return NET_ERR_OK;
        }

        // 进入正常的IP处理过程
        err = ip_normal_in(netif, full_buf, src, dest);
        if (err < 0) {
            dbg_warning(DBG_IP,"ip frag in error. err=%d\n", err);
            pktbuf_free(full_buf);  // 释放这里要注意
            return NET_ERR_OK;
        }
    }

    display_ip_frags();
    return NET_ERR_OK;
}

/**
 * @brief 通过分片的方式向网口发送数据包
 *
 * 注意：在没有实现分片输入处理的情况下，可尝试手动先将MTU值改小些，如256，甚至是128，从而能接收较大的IP包输入
 * 发送时即以分片的方式发送。测试时，使用ping产生较大的IP包，例如：ping -l 1330 192.168.254.2
 */
static net_err_t ip_frag_out(uint8_t protocol, ipaddr_t* dest,
                ipaddr_t* src, pktbuf_t* buf, ipaddr_t * next, netif_t * netif) {
    dbg_info(DBG_IP,"frag send an ip packet.\n");

    // 重新定位读取地址
    pktbuf_reset_acc(buf);

    // 一种效率更加高效的方式是将输入buf中的数据链进行分割，然后发送
    // 不过这种会麻烦一些，还需要提供buf分割函数，这里也改，挺麻烦的，所以这里就不做这种支持
    int offset = 0;
    int total = buf->total_size;        // 不包含IP包头的总大小
    while (total) {
        // 计算当前的大小，不含IP包头。本协议没有使用IP包头，不考虑选项的处理
        int curr_size = total;
        if (curr_size > netif->mtu - sizeof(ipv4_hdr_t)) {
            curr_size = netif->mtu - sizeof(ipv4_hdr_t);
        }

        // 非最后一块，将IP数据包负载部分大小调整为8字节对齐
        // 在以太网上，除IP包头外，数据负载为1480字节，是对齐的，但其它不一定对齐
        if (curr_size < total) {
            curr_size &= ~0x7;
        }

        // 1.为分片发送分配空间, 不考虑选项数据
        pktbuf_t * dest_buf = pktbuf_alloc(curr_size + sizeof(ipv4_hdr_t));
        if (!dest_buf) {
            dbg_error(DBG_IP,"alloc buf for frag send failed.\n");
            return NET_ERR_MEM;
        }

        // 2.写IP包头
        ipv4_pkt_t * pkt = (ipv4_pkt_t *)pktbuf_data(dest_buf);
        pkt->hdr.shdr_all = 0;       // 先清零0
        pkt->hdr.version = NET_VERSION_IPV4;
        set_header_size(pkt, sizeof(ipv4_hdr_t));
        pkt->hdr.total_len = dest_buf->total_size;
        pkt->hdr.id = packet_id;         //分片发送，该id保持不变
        pkt->hdr.frag_all = 0;           // 暂时清0分片配置
        pkt->hdr.ttl = NET_IP_DEF_TTL;
        pkt->hdr.protocol = protocol;
        pkt->hdr.hdr_checksum = 0;
        if (!src || ipaddr_is_any(src)) {
            // 未指定源地址，则使用网卡的地址
            ipaddr_to_buf(&netif->ipaddr, pkt->hdr.src_ip);
        } else {
            ipaddr_to_buf(src, pkt->hdr.src_ip);
        }
        ipaddr_to_buf(dest, pkt->hdr.dest_ip);

        // 调整分片偏移
        pkt->hdr.offset = offset >> 3;      // 8字节为单位
        pkt->hdr.more = total > curr_size; // 是否还有后续分片

        // 3. 拷贝数据区, 注意curr_size是包含头部的大小
        pktbuf_seek(dest_buf, sizeof(ipv4_hdr_t));
        net_err_t err = pktbuf_copy(dest_buf, buf, curr_size);
        if (err < 0) {
            dbg_error(DBG_IP,"frag copy failed. error = %d.\n", err);
            pktbuf_free(dest_buf);
            return err;
        }

        // 减去头部，这样就能释放出一些空间出来，避免出现完整的两条链
        // 实测发现，确实能有效地减少内存使用
        pktbuf_remove_header(buf, curr_size);
        pktbuf_reset_acc(buf);

        // 大小端处理
        iphdr_htons(pkt);

        // 注意，重新设置位置, 生产校验和
        pktbuf_seek(dest_buf, 0);
        pkt->hdr.hdr_checksum = pktbuf_checksum16(dest_buf, ipv4_hdr_size(pkt), 0, 1);

        // 4.发送数据包
        display_ip_packet((ipv4_pkt_t*)pkt);

        // 向下一跳发送数据包
        err = netif_out(netif, next, dest_buf);
        if (err < 0) {
            dbg_warning(DBG_IP,"ip send error. err = %d\n", err);
            pktbuf_free(dest_buf);
            return err;
        }

        // 调整位置及偏移
        total -= curr_size;
        offset += curr_size;
    }

    packet_id++;
    pktbuf_free(buf);
    return NET_ERR_OK;
}

/**
 * @brief 发送一个IP数据包
 *
 * 具体发送时，会根据是否超出MTU值，来进行分片发送。
 */
net_err_t ipv4_out(uint8_t protocol, ipaddr_t* dest, ipaddr_t * src, pktbuf_t* buf) {
    dbg_info(DBG_IP,"send an ip packet.\n");

    // 为目标ip选择路径，当前目标地址不一定是马上要发的地址
    rentry_t* rt = rt_find(dest);
    if (rt == (rentry_t *)0) {
        dbg_error(DBG_IP,"send failed. no route.");
        return NET_ERR_UNREACH;
    }
    ipaddr_t next_hop;
    if (ipaddr_is_any(&rt->next_hop)) {
        ipaddr_copy(&next_hop, dest);
    } else {
        ipaddr_copy(&next_hop, &rt->next_hop);
    }

    // 接口设置了MTU，但是MTU比较小，此时通过分片方式发送
    if (rt->netif->mtu && ((buf->total_size + sizeof(ipv4_hdr_t)) > rt->netif->mtu)) {
        // 允许分片，且超出MTU，则进行分片发送
        net_err_t err = ip_frag_out(protocol, dest, src, buf, &next_hop, rt->netif);
        if (err < 0) {
            dbg_warning(DBG_IP, "send ip frag packet failed. error = %d\n", err);
            return err;
        }
        return NET_ERR_OK;
    }

    // 调整读写位置，预留IP包头，注意要连续存储
    net_err_t err = pktbuf_add_header(buf, sizeof(ipv4_hdr_t), 1);
    if (err < 0) {
        dbg_error(DBG_IP, "no enough space for ip header, curr size: %d\n", buf->total_size);
        return NET_ERR_SIZE;
    }

    // 构建IP数据包
    ipv4_pkt_t * pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    pkt->hdr.shdr_all = 0;
    pkt->hdr.version = NET_VERSION_IPV4;
    set_header_size(pkt, sizeof(ipv4_hdr_t));
    pkt->hdr.total_len = buf->total_size;
    pkt->hdr.id = packet_id++;        // 计算不断自增
    pkt->hdr.frag_all = 0;         //
    pkt->hdr.ttl = NET_IP_DEF_TTL;
    pkt->hdr.protocol = protocol;
    pkt->hdr.hdr_checksum = 0;
    if (!src || ipaddr_is_any(src)) {
        // 未指定源地址，则使用网卡的地址
        ipaddr_to_buf(&rt->netif->ipaddr, pkt->hdr.src_ip);
    } else {
        ipaddr_to_buf(src, pkt->hdr.src_ip);
    }
    ipaddr_to_buf(dest, pkt->hdr.dest_ip);

    // 大小端转换
    iphdr_htons(pkt);

    // 计算校验和
    pktbuf_reset_acc(buf);
    pkt->hdr.hdr_checksum = pktbuf_checksum16(buf, ipv4_hdr_size(pkt), 0, 1);

    // 开始发送
    display_ip_packet(pkt);
    err = netif_out(rt->netif, &next_hop, buf);
    if (err < 0) {
        dbg_warning(DBG_IP, "send ip packet failed. error = %d\n", err);
        return err;
    }

    return NET_ERR_OK;
}

/**
 * @brief 检查包是否有错误
 */
static net_err_t is_pkt_ok(ipv4_pkt_t* pkt, int size) {
    // 版本检查，只支持ipv4
    if (pkt->hdr.version != NET_VERSION_IPV4) {
        dbg_warning(DBG_IP, "invalid ip version, only support ipv4!\n");
        return NET_ERR_NOT_SUPPORT;
    }

    // 头部长度要合适
    int hdr_len = ipv4_hdr_size(pkt);
    if (hdr_len < sizeof(ipv4_hdr_t)) {
        dbg_warning(DBG_IP, "IPv4 header error: %d!", hdr_len);
        return NET_ERR_SIZE;
    }

    // 总长必须大于头部长，且<=缓冲区长
    // 有可能xbuf长>ip包长，因为底层发包时，可能会额外填充一些字节
    int total_size = x_ntohs(pkt->hdr.total_len);
    if ((total_size < sizeof(ipv4_hdr_t)) || (size < total_size)) {
        dbg_warning(DBG_IP, "ip packet size error: %d!\n", total_size);
        return NET_ERR_SIZE;
    }

    // 校验和为0时，即为不需要检查检验和
    if (pkt->hdr.hdr_checksum) {
        uint16_t c = checksum16(0, (uint16_t*)pkt, hdr_len, 0, 1);
        if (c != 0) {
            dbg_warning(DBG_IP, "Bad checksum: %0x(correct is: %0x)\n", pkt->hdr.hdr_checksum, c);
            return 0;
        }
    }

    return NET_ERR_OK;
}

/**
 * @brief IP输入处理, 处理来自netif的buf数据包（IP包，不含其它协议头）
 */
net_err_t ipv4_in(netif_t* netif, pktbuf_t* buf) {
    dbg_info(DBG_IP, "IP in!\n");

    // 调整包头位置，保证连续性，方便接下来的处理
    net_err_t err = pktbuf_set_cont(buf, sizeof(ipv4_hdr_t));
    if (err < 0) {
        dbg_error(DBG_IP, "adjust header failed. err=%d\n", err);
        return err;
    }

    // 预先做一些检查
    ipv4_pkt_t* pkt = (ipv4_pkt_t*)pktbuf_data(buf);
    if (is_pkt_ok(pkt, buf->total_size) != NET_ERR_OK) {
        dbg_warning(DBG_IP, "packet is broken. drop it.\n");
        return err;
    }

    // 预先进行转换，方便后面进行处理，不用再临时转换
    iphdr_ntohs(pkt);

    // buf的整体长度可能比ip包要长，比如比较小的ip包通过以太网发送时
    // 由于底层包的长度有最小的要求，因此可能填充一些0来满足长度要求
    // 因此这里将包长度进行了缩小
    err = pktbuf_resize(buf, pkt->hdr.total_len);
    if (err < 0) {
        dbg_error(DBG_IP, "ip packet resize failed. err=%d\n", err);
        return err;
    }

    // 判断IP数据包是否是给自己的，不是给自己的包不处理
    ipaddr_t dest_ip, src_ip;
    ipaddr_from_buf(&dest_ip, pkt->hdr.dest_ip);
    ipaddr_from_buf(&src_ip, pkt->hdr.src_ip);

    // 最简单的判断：与本机网口相同, 或者广播
    if (!ipaddr_is_match(&dest_ip, &netif->ipaddr, &netif->netmask)) {
        pktbuf_free(buf);
        return NET_ERR_UNREACH;
    }

    // 发给自己的包，正常处理
    // 处理分片包的输入：该包的特点，offset不为0，或者虽为0但是more_frag不为0
    if (pkt->hdr.offset || pkt->hdr.more) {
        err = ip_frag_in(netif, buf, &src_ip, &dest_ip);
    } else {
        err = ip_normal_in(netif, buf, &src_ip, &dest_ip);
    }

    return err;
}

/**
 * @brief 分片功能初始化
 */
static net_err_t frag_init(void) {
    // 列表初始化
    nlist_init(&frag_list);
    mblock_init(&frag_mblock, frag_array, sizeof(ip_frag_t), IP_FRAGS_MAX_NR, NLOCKER_NONE);

    // 初始化分片扫描定时器
    net_err_t err = net_timer_add(&frag_timer, "frag timer", frag_tmo, (void *)0,
                        IP_FRAG_SCAN_PERIOD * 1000, NET_TIMER_RELOAD);
    if (err < 0) {
        dbg_error(DBG_IP, "create frag timer failed.\n");
        return err;
    }

    return NET_ERR_OK;
}

/**
 * @brief IP模块初始化
 */
net_err_t ipv4_init(void) {
    dbg_info(DBG_IP,"init ip\n");

    // 分片初始化
    net_err_t err = frag_init();
    if (err < 0) {
        dbg_error(DBG_IP,"failed. err = %d", err);
        return err;
    }

    // 路由表初始化
    rt_init();

    dbg_info(DBG_IP,"done.");
    return NET_ERR_OK;
}
