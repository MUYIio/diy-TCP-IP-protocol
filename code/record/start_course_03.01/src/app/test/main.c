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

int main (void) {
	pcap_t * pcap = pcap_device_open(netdev0_phy_ip, netdev0_hwaddr);
	while (pcap) {
		static uint8_t buffer[1024];
		static int counter = 0;

		plat_printf("begin test: %d\n", counter++);
		for (int i =0; i < sizeof(buffer); i++) {
			buffer[i] = i;
		}

		if (pcap_inject(pcap, buffer, sizeof(buffer)) == -1) {
			plat_printf("pcap send: send packet failed %s\n", pcap_geterr(pcap));
			break;
		}

		sys_sleep(10);
	}

	printf("Hello, world");
	return 0;
}