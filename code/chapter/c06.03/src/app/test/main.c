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
#include "nlist.h"

/**
 * @brief 网络设备初始化
 */
net_err_t netdev_init(void) {    
    netif_pcap_open();
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
}

/**
 * @brief 基本测试
 */
void basic_test(void) {
    nlist_test();
}

/**
 * @brief 测试入口
 */
int main (void) {
    // 初始化协议栈
    net_init();

    // 基础测试
    basic_test();

    // 初始化网络接口
    netdev_init();
    
    // 启动协议栈
    net_start();

    while (1) {
        sys_sleep(10);
    }
}
