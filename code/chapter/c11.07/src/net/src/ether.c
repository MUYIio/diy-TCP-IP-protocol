/**
 * @file ether.h
 * @author lishutong (527676163@qq.com)
 * @brief 以太网协议支持，不包含ARP协议处理
 * @version 0.1
 * @date 2022-10-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string.h>
#include "netif.h"
#include "ether.h"
#include "dbg.h"

/**
 * @brief 对指定接口设备进行以太网协议相关初始化
 */
static net_err_t ether_open(netif_t* netif) {
    return NET_ERR_OK;
}

/**
 * @brief 以太网的关闭
 */
static void ether_close(netif_t* netif) {
    
}

/**
 * 以太网输入包的处理
 */
static net_err_t ether_in(netif_t* netif, pktbuf_t* buf) {
    return NET_ERR_OK;
}

/**
 * 初始化以太网接口层
 */
net_err_t ether_init(void) {
    static const link_layer_t link_layer = {
        .type = NETIF_TYPE_ETHER,
        .open = ether_open,
        .close = ether_close,
        .in = ether_in,
        // .out = ether_out,
    };

    dbg_info(DBG_ETHER, "init ether");

    // 注册以太网驱链接层接口
    net_err_t err = netif_register_layer(NETIF_TYPE_ETHER, &link_layer);
    if (err < 0) {
        dbg_info(DBG_ETHER, "error = %d", err);
        return err;
    }

    dbg_info(DBG_ETHER, "done.");
    return NET_ERR_OK;
}
