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

/**
 * 数据包接收线程，不断地收数据包
 */
void recv_thread(void* arg) {
    plat_printf("recv thread is running...\n");

    while (1) {
        sys_sleep(1);
        exmsg_netif_in((netif_t *)0);
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
net_err_t netif_pcap_open(void) {
    sys_thread_create(xmit_thread, (void *)0);
    sys_thread_create(recv_thread, (void *)0);
    return NET_ERR_OK;
}
#endif