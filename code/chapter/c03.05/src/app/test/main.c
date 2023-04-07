/**
 * @file main.c
 * @author lishutong (527676163@qq.com)
 * @brief 测试主程序，完成一些简单的测试主程序
 * @version 0.1
 * @date 2022-10-23
 * 
 * @copyright Copyright (c) 2022
 * @note 该源码配套相应的视频课程，请见源码仓库下面的README.md
 */
#include <string.h>
#include "netif_pcap.h"
#include "sys_plat.h"

static sys_sem_t sem;
static sys_mutex_t mutex;

static int count;           // 计数器

/**
 * 线程1
 */
void thread1_entry (void* arg) {
    // count的值大一些，效果才显著
    for (int i = 0; i < 10000; i++) {
        sys_mutex_lock(mutex);
        count++;
        sys_mutex_unlock(mutex);
    }
    plat_printf("thread 1: count = %d\n", count);

    while (1) {
        plat_printf("this is thread 1: %s\n", (char *)arg);
        sys_sem_notify(sem);
        sys_sleep(1000);
    }
}

/**
 * 线程1
 */
void thread2_entry(void* arg) {
    // 注意循环的次数要和上面的一样
    for (int i = 0; i < 10000; i++) {
        sys_mutex_lock(mutex);
        count--;
        sys_mutex_unlock(mutex);
    }
    plat_printf("thread 2: count = %d\n", count);

    while (1) {
        sys_sem_wait(sem, 0);
        plat_printf("this is thread 2: %s\n", (char *)arg);
    }
}

/**
 * @brief 测试入口
 */
int main (void) {
    // 注意放在线程的创建的前面，以便线程运行前就准备好
    sem = sys_sem_create(0);
    mutex = sys_mutex_create();

    sys_thread_create(thread1_entry, "AAAA");
    sys_thread_create(thread2_entry, "BBBB");

    // 打开物理网卡，设置好硬件地址
    pcap_t * pcap = pcap_device_open(netdev0_phy_ip, netdev0_hwaddr);

    while (pcap) {
        static uint8_t buffer[1024];
        static int count = 0;
        struct pcap_pkthdr* pkthdr;
        const uint8_t* pkt_data;

        plat_printf("begin test: %d\n", count++);

        // 以下测试程序，读取网络上的广播包，然后再发送出去.
        // 读取数据包
        if (pcap_next_ex(pcap, &pkthdr, &pkt_data) != 1) {
            continue;
        }

        // 稍微修改一下，再发送
        int len = pkthdr->len > sizeof(buffer) ? sizeof(buffer) : pkthdr->len;
        plat_memcpy(buffer, pkt_data, len);
        buffer[0] = 1;
        buffer[1] = 2;

        // 发送数据包
        if (pcap_inject(pcap, buffer, len) == -1) {
            plat_printf("pcap send: send packet failed!:%s\n", pcap_geterr(pcap));
            plat_printf("pcap send: pcaket size %d\n", (int)len);
            break;
        }

    }
}
