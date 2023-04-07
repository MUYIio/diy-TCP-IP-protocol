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
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "dbg.h"

static sys_sem_t sem;
static sys_mutex_t mutex;

static int count;           // 计数器

static char buffer[100];
static sys_sem_t read_sem;      // 读信号量
static sys_sem_t write_sem;     // 写信号量
static int write_idx, read_idx, total;  // 写索引和读索引，总计数

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

   // 不断读取数据
    for (int i = 0; i < 2 * sizeof(buffer); i++) {
        // 等待有数据，即读信号量中的数据资源计数不为0
        sys_sem_wait(read_sem, 0);

        // 取出一个数据
        unsigned char data = buffer[read_idx++];
        if (read_idx >= sizeof(buffer)) {
            read_idx = 0;
        } 

        // 显示当前读的内容
        plat_printf("thread 1 read data=%d\n", data);

        // 调整总计数
        sys_mutex_lock(mutex);
        total--;
        sys_mutex_unlock(mutex);

        // 发信号给写线程，通知可以进行写了
        sys_sem_notify(write_sem);

        // 延时一会儿，让读取的速度慢一些
        sys_sleep(100);
    }

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

    // 连续写200次，由于中间没有printf、延时，所以写的速度是比较快的
    for (int i = 0; i < 2 * sizeof(buffer); i++) {
        // 写之前要等有可用的写空间，看看写信号量中空闲资源的数量不为0
        sys_sem_wait(write_sem, 0);

        // 不为0则将数据写入
        buffer[write_idx++] = i;
        if (write_idx >= sizeof(buffer)) {
            write_idx = 0;
        } 
        // 显示当前写的内容，方便观察
        plat_printf("thread 2 write data=%d\n", i);

        // 上锁，保护total的资源计数
        sys_mutex_lock(mutex);
        total++;
        sys_mutex_unlock(mutex);

        // 通知另一个线程可以进行读取，但具体什么时候读则不确定
        sys_sem_notify(read_sem);
    }
    sys_sleep(100000);      // 加点延时，避免后面的代码干扰

    while (1) {
        sys_sem_wait(sem, 0);
        plat_printf("this is thread 2: %s\n", (char *)arg);
    }
}

#include "netif_pcap.h"

/**
 * @brief 网络设备初始化
 */
net_err_t netdev_init(void) {    
    netif_pcap_open();
    return NET_ERR_OK;
}

/**
 * @brief 测试入口
 */
int main (void) {
    // 调试输出
    dbg_info("dbg_info");

    // 初始化协议栈
    net_init();

    // 初始化网络接口
    netdev_init();
    
    // 启动协议栈
    net_start();

    while (1) {
        sys_sleep(10);
    }
}
