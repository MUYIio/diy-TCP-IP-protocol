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
 * 检查包是否有错误
 */
static net_err_t is_pkt_ok(ether_pkt_t * frame, int total_size) {
    if (total_size > (sizeof(ether_hdr_t) + ETH_MTU)) {
        dbg_warning(DBG_ETHER, "frame size too big: %d", total_size);
        return NET_ERR_SIZE;
    }

    // 虽然以太网规定最小60字节，但是底层驱动收到可能小于这么多
    if (total_size < (sizeof(ether_hdr_t))) {
        dbg_warning(DBG_ETHER, "frame size too small: %d", total_size);
        return NET_ERR_SIZE;
    }

    return NET_ERR_OK;
}

/**
 * 以太网输入包的处理
 */
net_err_t ether_in(netif_t* netif, pktbuf_t* buf) {
    dbg_info(DBG_ETHER, "ether in:");

    // 似乎没什么必要，必须是连续的，但清空是加上。
    pktbuf_set_cont(buf, sizeof(ether_hdr_t));

    ether_pkt_t* pkt = (ether_pkt_t*)pktbuf_data(buf);

    // 检查包的正确性
    net_err_t err;
    if ((err = is_pkt_ok(pkt, buf->total_size)) != NET_ERR_OK) {
        dbg_error(DBG_ETHER, "ether pkt error");
        return err;
    }

    // 做点什么？？后面再加

    pktbuf_free(buf);
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
