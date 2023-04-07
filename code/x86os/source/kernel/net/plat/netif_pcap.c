/**
 * @brief 使用pcap_device创建的虚拟网络接口
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#if defined(NET_DRIVER_PCAP)

#include "dbg.h"
#include "net_plat.h"
#include "exmsg.h"
#include "netif.h"
#include "netif_pcap.h"
#include "ether.h"
#include "sys.h"

extern int load_pcap_lib(void);

/**
 * 根据ip地址查找本地网络接口列表，找到相应的名称
 */
int pcap_find_device(const char* ip, char* name_buf) {
    struct in_addr dest_ip;

    inet_pton(AF_INET, ip, &dest_ip);

    // 获取所有的接口列表
    char err_buf[PCAP_ERRBUF_SIZE];
    pcap_if_t* pcap_if_list = NULL;
    int err = pcap_findalldevs(&pcap_if_list, err_buf);
    if (err < 0) {
        pcap_freealldevs(pcap_if_list);
        return -1;
    }

    // 遍历列表
    pcap_if_t* item;
    for (item = pcap_if_list; item != NULL; item = item->next) {
        if (item->addresses == NULL) {
            continue;
        }

        // 查找地址
        for (struct pcap_addr* pcap_addr = item->addresses; pcap_addr != NULL; pcap_addr = pcap_addr->next) {
            // 检查ipv4地址类型
            struct sockaddr* sock_addr = pcap_addr->addr;
            if (sock_addr->sa_family != AF_INET) {
                continue;
            }

            // 地址相同则返回
            struct sockaddr_in* curr_addr = ((struct sockaddr_in*)sock_addr);
            if (curr_addr->sin_addr.s_addr == dest_ip.s_addr) {
                strcpy(name_buf, item->name);
                pcap_freealldevs(pcap_if_list);
                return 0;
            }
        }
    }

    pcap_freealldevs(pcap_if_list);
    return -1;
}

/*
 * 显示所有的网络接口列表，在出错时被调用
 */
int pcap_show_list(void) {
    char err_buf[PCAP_ERRBUF_SIZE];
    pcap_if_t* pcapif_list = NULL;
    int count = 0;

    // 查找所有的网络接口
    int err = pcap_findalldevs(&pcapif_list, err_buf);
    if (err < 0) {
        fprintf(stderr, "scan net card failed: %s\n", err_buf);
        pcap_freealldevs(pcapif_list);
        return -1;
    }

    printf("net card list: \n");

    // 遍历所有的可用接口，输出其信息
    for (pcap_if_t* item = pcapif_list; item != NULL; item = item->next) {
        if (item->addresses == NULL) {
            continue;
        }

        // 显示ipv4地址
        for (struct pcap_addr* pcap_addr = item->addresses; pcap_addr != NULL; pcap_addr = pcap_addr->next) {
            char str[INET_ADDRSTRLEN];
            struct sockaddr_in* ip_addr;

            struct sockaddr* sockaddr = pcap_addr->addr;
            if (sockaddr->sa_family != AF_INET) {
                continue;
            }

            ip_addr = (struct sockaddr_in*)sockaddr;
            char * name = item->description;
            if (name == NULL) {
                name = item->name;
            }
            printf("%d: IP:%s name: %s, \n",
                count++,
                name ? name : "unknown",
                inet_ntop(AF_INET, &ip_addr->sin_addr, str, sizeof(str))
            );
            break;
        }
    }

    pcap_freealldevs(pcapif_list);

    if ((pcapif_list == NULL) || (count == 0)) {
        fprintf(stderr, "error: no net card!\n");
        return -1;
    }

    printf("no net card found, check system configuration\n");
    return 0;
}

/**
 * 打开pcap设备接口
 */
pcap_t * pcap_device_open(const char* ip, const uint8_t* mac_addr) {
    // 加载pcap库
    if (load_pcap_lib() < 0) {
        fprintf(stderr, "load pcap lib error。在windows上，请课程提供的安装npcap.dll\n");
        return (pcap_t *)0;
    }

    // 利用上层传来的ip地址，
    char name_buf[256];
    if (pcap_find_device(ip, name_buf) < 0) {
        fprintf(stderr, "pcap find error: no net card has ip: %s. \n", ip);
        pcap_show_list();
        return (pcap_t*)0;
    }

    // 根据名称获取ip地址、掩码等
    char err_buf[PCAP_ERRBUF_SIZE];
    bpf_u_int32 mask;
    bpf_u_int32 net;
    if (pcap_lookupnet(name_buf, &net, &mask, err_buf) == -1) {
        printf("pcap_lookupnet error: no net card: %s\n", name_buf);
        net = 0;
        mask = 0;
    }

    // 打开设备
    pcap_t * pcap = pcap_create(name_buf, err_buf);
    if (pcap == NULL) {
        fprintf(stderr, "pcap_create: create pcap failed %s\n net card name: %s\n", err_buf, name_buf);
        fprintf(stderr, "Use the following:\n");
        pcap_show_list();
        return (pcap_t*)0;
    }

    if (pcap_set_snaplen(pcap, 65536) != 0) {
        fprintf(stderr, "pcap_open: set none block failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    if (pcap_set_promisc(pcap, 1) != 0) {
        fprintf(stderr, "pcap_open: set none block failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    if (pcap_set_timeout(pcap, 0) != 0) {
        fprintf(stderr, "pcap_open: set none block failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 非阻塞模式读取，程序中使用查询的方式读
    if (pcap_set_immediate_mode(pcap, 1) != 0) {
        fprintf(stderr, "pcap_open: set im block failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    if (pcap_activate(pcap) != 0) {
        fprintf(stderr, "pcap_open: active failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    if (pcap_setnonblock(pcap, 0, err_buf) != 0) {
        fprintf(stderr, "pcap_open: set none block failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 只捕获发往本接口与广播的数据帧。相当于只处理发往这张网卡的包
    char filter_exp[256];
    struct bpf_program fp;
    sprintf(filter_exp,
        "(ether dst %02x:%02x:%02x:%02x:%02x:%02x or ether broadcast) and (not ether src %02x:%02x:%02x:%02x:%02x:%02x)",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    if (pcap_compile(pcap, &fp, filter_exp, 0, net) == -1) {
        printf("pcap_open: couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(pcap));
        return (pcap_t*)0;
    }
    if (pcap_setfilter(pcap, &fp) == -1) {
        printf("pcap_open: couldn't install filter %s: %s\n", filter_exp, pcap_geterr(pcap));
        return (pcap_t*)0;
    }
    return pcap;
}

/**
 * 数据包接收线程，不断地收数据包
 */
void recv_thread(void* arg) {
    plat_printf("recv thread is running...\n");

    netif_t* netif = (netif_t*)arg;
    pcap_t* pcap = (pcap_t*)netif->ops_data;

    while (1) {
        // 获取一个数据包
        // 1 - 成功读取数据包, 0 - 没有数据包，其它值-出错
        struct pcap_pkthdr* pkthdr;
        const uint8_t* pkt_data;
        if (pcap_next_ex(pcap, &pkthdr, &pkt_data) != 1) {
            continue;
        }

        // 将data转变为缓存结构
        pktbuf_t* buf = pktbuf_alloc(pkthdr->len);
        if (buf == (pktbuf_t*)0) {
            dbg_warning(DBG_NETIF, "buf == NULL");
            continue;
        }

        // 写入数据链表中
        pktbuf_write(buf, (uint8_t*)pkt_data, pkthdr->len);

        // 写入接收队列，可能写失败，比如队列已经满了
        if (netif_put_in(netif, buf, 0) < 0) {
            dbg_warning(DBG_NETIF, "netif %s in_q full", netif->name);
            pktbuf_free(buf); // 释放掉
            continue;
        }
    }
}

/**
 * 模拟硬件发送线程
 */
void xmit_thread(void* arg) {
    plat_printf("xmit thread is running...\n");

    // 最大1514， 此为以太网MTU（1500）+包头（14）字节
    static uint8_t rw_buffer[1514];
    netif_t* netif = (netif_t*)arg;
    pcap_t* pcap = (pcap_t*)netif->ops_data;

    while (1) {
        // 从发送队列取数据包
        pktbuf_t* buf = netif_get_out(netif, 0);
        if (buf == (pktbuf_t*)0) {
            continue;
        }

        // 写入临时缓存后，将数据发送出去
        int total_size = buf->total_size;
        memset(rw_buffer, 0, sizeof(rw_buffer));
        pktbuf_read(buf, rw_buffer, total_size);
        pktbuf_free(buf);   // 拷完后释放

        if (pcap_inject(pcap, rw_buffer, total_size) == -1) {
            fprintf(stderr, "pcap send: send packet failed!:%s\n", pcap_geterr(pcap));
            fprintf(stderr, "pcap send: pcaket size %d\n", total_size);
            continue;
        }
    }
}

/**
 * pcap设备打开
 * @param netif 打开的接口
 * @param driver_data 传入的驱动数据
 */
static net_err_t netif_pcap_open(struct _netif_t* netif, void* ops_data) {
    // 打开pcap设备
    pcap_data_t* dev_data = (pcap_data_t*)ops_data;
    pcap_t * pcap = pcap_device_open(dev_data->ip, dev_data->hwaddr);
    if (pcap == (pcap_t*)0) {
        dbg_error(DBG_NETIF, "pcap open failed! name: %s\n", netif->name);
        return NET_ERR_IO;
    }
    netif->ops_data = pcap;

    netif->type = NETIF_TYPE_ETHER;         // 以太网类型
    netif->mtu = ETH_MTU;            // 1500字节
    netif_set_hwaddr(netif, dev_data->hwaddr, NETIF_HWADDR_SIZE);

    sys_thread_create(xmit_thread, netif);
    sys_thread_create(recv_thread, netif);
    return NET_ERR_OK;
}

/**
 * 关闭pcap网络接口
 * @param netif 待关闭的接口
 */
static void netif_pcap_close(struct _netif_t* netif) {
    pcap_t* pcap = (pcap_t*)netif->ops_data;
    pcap_close(pcap);

    // todo: 关闭线程
}

/**
 * 向接口发送命令
 */
static net_err_t netif_pcap_xmit (struct _netif_t* netif) {
    return NET_ERR_OK;
}

/**
 * pcap驱动结构
 */
const netif_ops_t netdev_ops = {
    .open = netif_pcap_open,
    .close = netif_pcap_close,
    .xmit = netif_pcap_xmit,
};
#endif