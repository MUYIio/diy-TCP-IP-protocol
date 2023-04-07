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

static sys_sem_t sem;
static sys_mutex_t mutex;
static int count;

static char buffer[100];
static int write_indx, read_index;

void thread1_entry (void * arg) {
	for (int i = 0; i < 2 * sizeof(buffer); i++) {
		char data = buffer[read_index++];

		if (read_index >= sizeof(buffer)) {
			read_index = 0;
		}

		plat_printf("thread 1: read data=%d\n", data);
		sys_sleep(200);
	}

	while (1) {
		plat_printf("this is thread1: %s\n", (char *)arg);
		sys_sleep(1000);
		sys_sem_notify(sem);
		sys_sleep(1000);
	}
}

void thread2_entry (void * arg) {
	for (int i = 0; i < 2 * sizeof(buffer); i++) {
		buffer[write_indx++] = i;

		if (write_indx >= sizeof(buffer)) {
			write_indx = 0;
		}

		plat_printf("thread 2 : write data = %d\n", i);

		sys_sleep(100);
	}

	while (1) {
		sys_sem_wait(sem, 0);
		plat_printf("this is thread2: %s\n", (char *)arg);
	}
}

int main (void) {
	sem = sys_sem_create(0);
	mutex = sys_mutex_create();
	
	sys_thread_create(thread1_entry, "AAAA");
	sys_thread_create(thread2_entry, "BBBB");

	pcap_t * pcap = pcap_device_open(netdev0_phy_ip, netdev0_hwaddr);
	while (pcap) {
		static uint8_t buffer[1024];
		static int counter = 0;
		struct pcap_pkthdr * pkthdr;
		const uint8_t * pkt_data;

		plat_printf("begin test: %d\n", counter++);
		for (int i =0; i < sizeof(buffer); i++) {
			buffer[i] = i;
		}

		if (pcap_next_ex(pcap, &pkthdr, &pkt_data) != 1) {
			continue;
		}

		int len = pkthdr->len > sizeof(buffer) ?sizeof(buffer) : pkthdr->len;
		plat_memcpy(buffer, pkt_data, len);
		buffer[0] = 1;
		buffer[1] = 2;

		if (pcap_inject(pcap, buffer, len) == -1) {
			plat_printf("pcap send: send packet failed %s\n", pcap_geterr(pcap));
			break;
		}
	}

	printf("Hello, world");
	return 0;
}