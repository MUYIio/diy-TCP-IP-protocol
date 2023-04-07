/**
 * @file loop.c
 * @author lishutong(527676163@qq.com)
 * @brief 回环接口
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * 环回接口是一种虚拟接口，可以在没有物理机器或网络连接的情况下，实现本机
 * 上不同应用程序之前通过TCP/IP协议栈通信
 */
#include "netif.h"
#include "dbg.h"

/**
 * @brief 打开环回接口
 */
static net_err_t loop_open(netif_t* netif, void* ops_data) {
    netif->type = NETIF_TYPE_LOOP;
    return NET_ERR_OK;
}

/**
 * @brief 关闭环回接口
 */
static void loop_close (struct _netif_t* netif) {
}

/**
 * @brief 启动数据包的发送
 */
static net_err_t loop_xmit (struct _netif_t* netif) {
    return NET_ERR_OK;
}

/**
 * @brief 环回接口驱动
 */
static const netif_ops_t loop_driver = {
    .open = loop_open,
    .close = loop_close,
    .xmit = loop_xmit,
};

/**
 * @brief 初始化环回接口
 */
net_err_t loop_init(void) {
    dbg_info(DBG_NETIF, "init loop");

    // 打开接口
    netif_t * netif = netif_open("loop", &loop_driver, (void*)0);
    if (!netif) {
        dbg_error(DBG_NETIF, "open loop error");
        return NET_ERR_NONE;
    }
 
    dbg_info(DBG_NETIF, "init done");
    return NET_ERR_OK;;
}

