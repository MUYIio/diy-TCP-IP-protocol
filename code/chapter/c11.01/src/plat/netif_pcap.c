/**
 * @brief 使用pcap_device创建的虚拟网络接口
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#if defined(NET_DRIVER_PCAP)
#include "net_plat.h"
#include "exmsg.h"
#include "netif.h"
#include "netif_pcap.h"
#include "dbg.h"

/**
 * 数据包接收线程，不断地收数据包
 */
void recv_thread(void* arg) {
    plat_printf("recv thread is running...\n");

    while (1) {
        sys_sleep(1);
        //exmsg_netif_in((netif_t *)0);
    }
}

/**
 * 模拟硬件发送线程
 */
void xmit_thread(void* arg) {
    plat_printf("xmit thread is running...\n");

    while (1) {
        sys_sleep(1);
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
    netif->mtu = 1500;            // 1500字节
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
