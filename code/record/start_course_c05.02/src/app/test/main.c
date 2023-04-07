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
#include <stdio.h>
#include "sys_plat.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "dbg.h"

static sys_sem_t sem;
static sys_mutex_t mutex;
static int count;

static char buffer[100];
static int write_indx, read_index, total;
static sys_sem_t read_sem, write_sem;

void thread1_entry (void * arg) {
	for (int i = 0; i < 2 * sizeof(buffer); i++) {
		sys_sem_wait(read_sem, 0);

		char data = buffer[read_index++];

		if (read_index >= sizeof(buffer)) {
			read_index = 0;
		}

		sys_mutex_lock(mutex);
		total--;
		sys_mutex_unlock(mutex);

		sys_sem_notify(write_sem);

		plat_printf("thread 1: read data=%d\n", data);

		sys_sleep(100);
	}

	while (1) {
		plat_printf("this is thread1: %s\n", (char *)arg);
		sys_sleep(1000);
		sys_sem_notify(sem);
		sys_sleep(1000);
	}
}

void thread2_entry (void * arg) {
	sys_sleep(100);

	for (int i = 0; i < 2 * sizeof(buffer); i++) {
		sys_sem_wait(write_sem, 0);

		buffer[write_indx++] = i;

		if (write_indx >= sizeof(buffer)) {
			write_indx = 0;
		}

		sys_mutex_lock(mutex);
		total++;
		sys_mutex_unlock(mutex);

		plat_printf("thread 2 : write data = %d\n", i);

		sys_sem_notify(read_sem);
	}

	while (1) {
		sys_sem_wait(sem, 0);
		plat_printf("this is thread2: %s\n", (char *)arg);
	}
}

#include "netif_pcap.h"

net_err_t netdev_init (void) {
	netif_pcap_open();

	return NET_ERR_OK;
}

int main (void) {
	dbg_info(DBG_STYLE_ERROR"dbg_info");
	dbg_info(DBG_STYLE_RESET"dbg_info");
	dbg_info(DBG_STYLE_WARNING"dbg_info");

	net_init();

	net_start();

	netdev_init();

	while (1) {
		sys_sleep(10);
	}
	return 0;
}