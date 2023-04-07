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
#include "echo/udp_echo_client.h"
#include "echo/udp_echo_server.h"
#include "net.h"
#include "dbg.h"
#include "nlist.h"
#include "mblock.h"
#include "pktbuf.h"
#include "netif.h"
#include "tools.h"
#include "timer.h"
#include "ipv4.h"
#include "ping/ping.h"
#include "exmsg.h"
#include "ntp/ntp.h"
#include "tftp/tftp_client.h"
#include "tftp/tftp_server.h"
#include "http/httpd.h"


#include "netif_pcap.h"
pcap_data_t netdev0_data = { .ip = netdev0_phy_ip, .hwaddr = netdev0_hwaddr };
extern const netif_ops_t netdev_ops;

/**
 * @brief 网络设备初始化
 */
net_err_t netdev_init(void) {
    // 打开网络接口
    netif_t* netif = netif_open("netif 0", &netdev_ops, &netdev0_data);
    if (!netif) {
        fprintf(stderr, "netif open failed.");
        exit(-1);
    }

    // 生成相应的地址
    ipaddr_t ip, mask, gw;
    ipaddr_from_str(&ip, netdev0_ip);
    ipaddr_from_str(&mask, netdev0_mask);
    ipaddr_from_str(&gw, netdev0_gw);
    netif_set_addr(netif, &ip, &mask, &gw);

    // 激活网络接口
    netif_set_active(netif);

	// 测试发送
	// pktbuf_t * buf = pktbuf_alloc(131);		// 设置为奇数个
	// pktbuf_join(buf, pktbuf_alloc(45));
	// pktbuf_fill(buf, 0x34, buf->total_size);

	// ipaddr_t dest, src;
	// ipaddr_from_str(&dest, friend0_ip);
	// ipaddr_from_str(&src, netdev0_ip);
	// ipv4_out(0, &dest, &src, buf);

	// // 发送广播测试
	// ipaddr_from_str(&dest, "255.255.255.255");
	// buf = pktbuf_alloc(32);
	// pktbuf_fill(buf, 0xA5, buf->total_size);
	// netif_out(netif, &dest, buf);

    return NET_ERR_OK;
}

/**
 * @brief 测试结点
 */
typedef struct _tnode_t {
    int id;
    nlist_node_t node;
}tnode_t;

/**
 * @brief 链表访问测试
 */
void nlist_test (void) {
    #define NODE_CNT        8

    tnode_t node[NODE_CNT];
    nlist_t list;
    nlist_node_t * p;

    nlist_init(&list);

    // 头部插入
    for (int i = 0; i < NODE_CNT; i++) {
        node[i].id = i;
        nlist_insert_first(&list, &node[i].node);
    }

    // 遍历打印
    plat_printf("insert first\n");
    nlist_for_each(p, &list) {
        tnode_t * tnode = nlist_entry(p, tnode_t, node);
        plat_printf("id:%d\n", tnode->id);
    }

    // 头部移除
    plat_printf("remove first\n");
    for (int i = 0; i < NODE_CNT; i++) {
        p = nlist_remove_first(&list);
        plat_printf("id:%d\n", nlist_entry(p, tnode_t, node)->id);
   }

    // 尾部插入
    for (int i = 0; i < NODE_CNT; i++) {
        nlist_insert_last(&list, &node[i].node);
    }

    // 遍历打印
    plat_printf("insert last\n");
    nlist_for_each(p, &list) {
        tnode_t * tnode = nlist_entry(p, tnode_t, node);
        plat_printf("id:%d\n", tnode->id);
    }

    // 尾部移除
    plat_printf("remove last\n");
    for (int i = 0; i < NODE_CNT; i++) {
        p = nlist_remove_last(&list);
        plat_printf("id:%d\n", nlist_entry(p, tnode_t, node)->id);
   }

   // 插入到指定结点之后
    plat_printf("insert after\n");
    for (int i = 0; i < NODE_CNT; i++) {
        nlist_insert_after(&list, nlist_first(&list), &node[i].node);
    }

    // 遍历打印
    nlist_for_each(p, &list) {
        tnode_t * tnode = nlist_entry(p, tnode_t, node);
        plat_printf("id:%d\n", tnode->id);
    }
}

void mblock_test(void) {
    static uint8_t buffer[10][100];
	mblock_t blist;

	mblock_init(&blist, buffer, 100, 10, NLOCKER_THREAD);
	void* temp[10];

	// 从管理器中逐个分配内存块
	for (int i = 0; i < 10; i++) {
		temp[i] = mblock_alloc(&blist, 0);
		printf("block: %p, free count:%d\n", temp[i], mblock_free_cnt(&blist));
	}
	for (int i = 0; i < 10; i++) {
		mblock_free(&blist, temp[i]);
		printf("free count:%d\n", mblock_free_cnt(&blist));
	}

	mblock_destroy(&blist);
}

void pktbuf_test(void) {
	static uint16_t temp[1000];
	static uint16_t read_temp[1000];

    // 初始化数据空间
	for (int i = 0; i < 1024; i++) {
		temp[i] = i;
	}

    // 简单的分配和释放, 2000字节.注意打开pktbuf的显示，方便观察
	pktbuf_t * buf = pktbuf_alloc(2000);
	pktbuf_free(buf);

	// 添加头部空间
	buf = pktbuf_alloc(2000);

    // 要求连续的头部添加。最终可以到看，有些包的头部会有一些空间小于33
    // 由于空间不够，只能舍弃
	for (int i = 0; i < 16; i++) {
		pktbuf_add_header(buf, 33, 1);      // 连续的空间
	}
	for (int i = 0; i < 16; i++) {
		pktbuf_remove_header(buf, 33);      // 移除
	}

    // 与连续分配的要求相比，总的包数量小一些，且除第一个块外，其它
    // 块没有开头浪费的空间
	for (int i = 0; i < 16; i++) {
		pktbuf_add_header(buf, 33, 0);		// 非连续添加
	}
	for (int i = 0; i < 16; i++) {
		pktbuf_remove_header(buf, 33);
	}
	pktbuf_free(buf);

	// 大小的调整，先变大变小
	buf = pktbuf_alloc(0);  // 大小为0
	pktbuf_resize(buf, 32);
	pktbuf_resize(buf, 288);
	pktbuf_resize(buf, 4922);
	pktbuf_resize(buf, 1921);
	pktbuf_resize(buf, 288);
	pktbuf_resize(buf, 32);
	pktbuf_resize(buf, 0);
	pktbuf_free(buf);

	// 两个包的连接。在最终的显示结果中，可以看到两个包之间的连接交叉处
	buf = pktbuf_alloc(689);
	pktbuf_t * sbuf = pktbuf_alloc(892);
	pktbuf_join(buf, sbuf);
	pktbuf_free(buf);

	// 小包的连接测试并调整连续性.先合并一些小的包，以形成很多个小包的连接
    // 然后再调整连续性，可以使链的连接在不断变短
	buf = pktbuf_alloc(32);
	pktbuf_join(buf, pktbuf_alloc(4));
	pktbuf_join(buf, pktbuf_alloc(16));
	pktbuf_join(buf, pktbuf_alloc(54));
	pktbuf_join(buf, pktbuf_alloc(32));
	pktbuf_join(buf, pktbuf_alloc(38));
	pktbuf_set_cont(buf, 44);			// 合并成功，簇变短
	pktbuf_set_cont(buf, 60);			// 合并成功，簇变短
	pktbuf_set_cont(buf, 64);			// 合并成功，簇变短
	pktbuf_set_cont(buf, 128);			// 合并成功，簇变短
	pktbuf_set_cont(buf, 135);			// 失败，超过128
	pktbuf_free(buf);

	// 准备一些不同大小的包链，方便后面读写测试
	buf = pktbuf_alloc(32);
	pktbuf_join(buf, pktbuf_alloc(4));
	pktbuf_join(buf, pktbuf_alloc(16));
	pktbuf_join(buf, pktbuf_alloc(54));
	pktbuf_join(buf, pktbuf_alloc(32));
	pktbuf_join(buf, pktbuf_alloc(38));
	pktbuf_join(buf, pktbuf_alloc(512));

    // 读写测试。写超过1包的数据，然后读取
	pktbuf_reset_acc(buf);
	pktbuf_write(buf, (uint8_t *)temp, pktbuf_total(buf));      // 16位的读写
	plat_memset(read_temp, 0, sizeof(read_temp));
	pktbuf_reset_acc(buf);
	pktbuf_read(buf, (uint8_t*)read_temp, pktbuf_total(buf));
	if (plat_memcmp(temp, read_temp, pktbuf_total(buf)) != 0) {
		printf("not equal.");
		exit(-1);
	}

	// 定位读写，不超过1个块
	plat_memset(read_temp, 0, sizeof(read_temp));
	pktbuf_seek(buf, 18 * 2);
	pktbuf_read(buf, (uint8_t*)read_temp, 56);
	if (plat_memcmp(temp + 18, read_temp, 56) != 0) {
		printf("not equal.");
		exit(-1);
	}

    // 定位跨一个块的读写测试, 从170开始读，读56
	plat_memset(read_temp, 0, sizeof(read_temp));
	pktbuf_seek(buf, 85 * 2);
	pktbuf_read(buf, (uint8_t*)read_temp, 256);
	if (plat_memcmp(temp + 85, read_temp, 256) != 0) {
		printf("not equal.");
		exit(-1);
	}

	// 数据的复制
	pktbuf_t* dest = pktbuf_alloc(1024);
	pktbuf_seek(buf, 200);      // 从200处开始读
	pktbuf_seek(dest, 600);     // 从600处开始写
	pktbuf_copy(dest, buf, 122);    // 复制122个字节

    // 重新定位到600处开始读
	plat_memset(read_temp, 0, sizeof(read_temp));
	pktbuf_seek(dest, 600);
	pktbuf_read(dest, (uint8_t*)read_temp, 122);    // 读122个字节
	if (plat_memcmp(temp + 100, read_temp, 122) != 0) { // temp+100，实际定位到200字节偏移处
		printf("not equal.");
		exit(-1);
	}

	// 填充测试
	pktbuf_seek(dest, 0);
	pktbuf_fill(dest, 53, pktbuf_total(dest));

	plat_memset(read_temp, 0, sizeof(read_temp));
	pktbuf_seek(dest, 0);
	pktbuf_read(dest, (uint8_t*)read_temp, pktbuf_total(dest));
	for (int i = 0; i < pktbuf_total(dest); i++) {
		if (((uint8_t *)read_temp)[i] != 53) {
			printf("not equal.");
			exit(-1);
		}
	}

	pktbuf_free(dest);
	pktbuf_free(buf);       // 可以进去调试，在退出函数前看下所有块是否全部释放完毕
}

void timer0_proc(net_timer_t* timer, void * arg) {
	static int count = 1;
	printf("this is %s: %d\n", timer->name, count++);
}

void timer1_proc(net_timer_t* timer, void * arg) {
	static int count = 1;
	printf("this is %s: %d\n", timer->name, count++);
}

void timer2_proc(net_timer_t* timer, void * arg) {
	static int count = 1;
	printf("this is %s: %d\n", timer->name, count++);
}

void timer3_proc(net_timer_t* timer, void * arg) {
	static int count = 1;
	printf("this is %s: %d\n", timer->name, count++);
}

void timer_test(void) {
	static net_timer_t t0, t1, t2, t3;

	// 一次性定时器
	net_timer_add(&t0, "t0", timer0_proc, (void *)0, 200, 0);

	// 自动重载定时器
	net_timer_add(&t1, "t1", timer1_proc, (void *)0, 1000, NET_TIMER_RELOAD);
	net_timer_add(&t2, "t2", timer2_proc, (void *)0, 1000, NET_TIMER_RELOAD);
	net_timer_add(&t3, "t3", timer3_proc, (void *)0, 4000, NET_TIMER_RELOAD);
	net_timer_remove(&t1);
}

/**
 * @brief 基本测试
 */
void basic_test(void) {
    nlist_test();
    mblock_test();
    pktbuf_test();

	uint32_t v1 = x_ntohl(0x12345678);
	uint16_t v2 = x_ntohs(0x1234);

    timer_test();
}

void show_help (void) {
	printf("--------------- cmd list ------------------ \n");
	printf("1.ping dest(ip or name)\n");
	printf("2.time any\n");
	printf("3.tftp client\n");
	printf("4.tftp-get file_path\n");
    printf("5.get filename\n");
}

void download_test (const char * filename);


/**
 * @brief 测试入口
 */
int main (void) {
    // 初始化协议栈
    net_init();

    // 基础测试
    //basic_test();

    // 初始化网络接口
    netdev_init();

    // 启动协议栈
    net_start();

   // udp echo服务器
    udp_echo_server_start(2000);

    // udp echo客户端
    //udp_echo_client_start(friend0_ip, 1000);

    // 开启tftp服务
	tftpd_start("tftp", TFTP_DEF_PORT);

	//tftp_get(friend0_ip, TFTP_DEF_PORT, 1024, "tftpd32.ini");
	// tftp_put(friend0_ip, TFTP_DEF_PORT, 1024, "tftpd32.chm");
	// tftp_get(friend0_ip, TFTP_DEF_PORT, 1024, "2.exe");

    // tcp echo客户端
    //tcp_echo_client_start(friend0_ip, 2000);
	tcp_echo_server_start(2000);

	// http服务器
	httpd_start(HTTPD_DEFAULT_ROOT, HTTPD_DEFAULT_PORT);

    // 应用程序
	ping_t p;
	//ping_run(&p, friend0_ip, 4, 64, 1000);    	// 邻居测试
	//ping_run(&p, "8.8.8.8", 4, 64, 1000);    	// google的DNS服务器

	//int arg = 0x1234;
	//exmsg_func_exec(test_func, (void *)&arg);

	char cmd[32], param[32];
    while (1) {
		show_help();
        printf(">>");

		// 注意，这里的scanf对于\r\n的输入会导致只读取了\r，后面还会读取\n
        scanf("%s %s", cmd, param);

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
		} else if (strcmp(cmd, "get") == 0) {
    		download_test(param);
		}else {
			printf("unknown command\n");
		}
    }
}
