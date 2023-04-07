/**
 * @brief TCP/IP协议栈初始化
 *
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note 
 */
#include "net.h"
#include "net_plat.h"
#include "exmsg.h"
#include "pktbuf.h"
#include "dbg.h"
#include "netif.h"
#include "loop.h"
#include "ether.h"

/**
 * 协议栈初始化
 */
net_err_t net_init(void) {
    dbg_info(DBG_INIT, "init net...");

    // 各模块初始化
    net_plat_init();
    exmsg_init();
    pktbuf_init();
    netif_init();
    ether_init();

    // 环回接口初始化
    loop_init();
    return NET_ERR_OK;
}

/**
 * 启动协议栈
 */
net_err_t net_start(void) {
    // 启动消息传递机制
    exmsg_start();

    dbg_info(DBG_INIT, "net is running.");
    return NET_ERR_OK;
}