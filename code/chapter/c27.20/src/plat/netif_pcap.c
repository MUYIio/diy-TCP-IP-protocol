/**
 * @brief 使用pcap_device创建的虚拟网络接口
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#if defined(NET_DRIVER_PCAP)
#include "ether.h"
#include "dbg.h"
#include "net_plat.h"
#include "exmsg.h"
#include "netif.h"
#include "netif_pcap.h"

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
        plat_memset(rw_buffer, 0, sizeof(rw_buffer));
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