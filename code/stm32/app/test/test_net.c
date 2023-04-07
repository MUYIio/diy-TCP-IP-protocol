#include "os_task.h"
#include "os_core.h"

#if 1
#include "net.h"
#include "netif.h"
#include "ping/ping.h"
#include "http/httpd.h"
#include "tftp/tftp_client.h"
#include "tftp/tftp_server.h"
#include "ntp/ntp.h"
#include "echo/tcp_echo_server.h"
#include "echo/udp_echo_server.h"

#include "netif_disc.h"
#include "LAN8720.h"
#include "usart.h"

#include "ff.h"  
#include "sdio_sdcard.h"    

static netif_disc_data_t disc_data = {
    .hwaddr = netdev0_hwaddr,
};

/**
 * @brief 网络设备初始化
 */
net_err_t netdev_init(void) {	
    // 打开网络接口
    netif_t* netif = netif_open("enc28j60", &netif_disc_ops, &disc_data);
    if (!netif) {
        plat_printf("netif %d open failed.", 0);
        return NET_ERR_IO;
    }

    // 生成相应的地址
    ipaddr_t ip, mask, gw;
    ipaddr_from_str(&ip, "192.168.0.250");
    ipaddr_from_str(&mask, "255.255.255.0");
    ipaddr_from_str(&gw, "192.168.0.1");
    netif_set_addr(netif, &ip, &mask, &gw);
    netif_set_default(netif);
    netif_set_active(netif);

    return NET_ERR_OK;
}

static FATFS fs;

void os_app_init (void) {
  	f_mount(&fs,"0:",1);

    net_init();
    netdev_init();
    net_start();   
}

void os_app_loop(void) {
	static ping_t p;
    static char cmd[32], param[32];

    httpd_start("htdocs", HTTPD_DEFAULT_PORT);
	//ping_run(&p, "127.0.0.1", 4, 1000, 1000);
    tftpd_start("file", TFTP_DEF_PORT);
    //tftp_get("127.0.0.1", TFTP_DEF_PORT, 512, "1234");
    tcp_echo_server_start(5000);
    udp_echo_server_start(5000);
	
    while (1) {        
        plat_printf(">>");

				// 注意，这里的scanf对于\r\n的输入会导致只读取了\r，后面还会读取\n
        scanf("%s%s", cmd, param);

        if (strcmp(cmd, "ping") == 0) {
            ping_run(&p, param, 4, 1000, 1000);
        } else if (strcmp(cmd, "tftp-get") == 0) {
			tftp_get(friend0_ip, TFTP_DEF_PORT, TFTP_DEF_BLKSIZE, param);
        } else if (strcmp(cmd, "tftp-put") == 0) {
			tftp_put(friend0_ip, TFTP_DEF_PORT, TFTP_DEF_BLKSIZE, param);
		} else if (strcmp(cmd, "tftp") == 0) {
			// tftp客户端
    		tftp_start(friend0_ip, TFTP_DEF_PORT);
		} else if (strcmp(cmd, "time") == 0) {
			struct tm * t = ntp_time();
			if (t) {
				printf("time: %d-%d-%d %d:%d:%d\n",
					t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
					t->tm_hour, t->tm_min, t->tm_sec);
			}
		}else {
			printf("unknown command\n");
		}
    }
}

#endif

