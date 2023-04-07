/**
 * @file netif.c
 * @author lishutong(527676163@qq.com)
 * @brief 网络接口层代码
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * 该接口层代码负责将所有的网络接口统一抽像为netif结构，并提供统一的接口进行处理
 */
#include <string.h>
#include <stdlib.h>
#include "netif.h"
#include "net.h"
#include "dbg.h"
#include "fixq.h"
#include "exmsg.h"
#include "mblock.h"

static netif_t netif_buffer[NETIF_DEV_CNT];     // 网络接口的数量
static mblock_t netif_mblock;                   // 接口分配结构
static nlist_t netif_list;               // 网络接口列表
static netif_t * netif_default;          // 缺省的网络列表

/**
 * @brief 网络接口层初始化
 */
net_err_t netif_init(void) {
    dbg_info(DBG_NETIF, "init netif");

    // 建立接口列表
    nlist_init(&netif_list);
    mblock_init(&netif_mblock, netif_buffer, sizeof(netif_t), NETIF_DEV_CNT, NLOCKER_NONE);

    // 设置缺省接口
    netif_default = (netif_t *)0;
    
    dbg_info(DBG_NETIF, "init done.\n");
    return NET_ERR_OK;
}

/**
 * @brief 打开指定的网络接口
 */
netif_t* netif_open(const char* dev_name) {
    // 分配一个网络接口
    netif_t*  netif = (netif_t *)mblock_alloc(&netif_mblock, -1);
    if (!netif) {
        dbg_error(DBG_NETIF, "no netif");
        return (netif_t*)0;
    }

    // 设置各种缺省值, 这些值有些将被驱动处理，有些将被上层netif_xxx其它函数设置
    ipaddr_set_any(&netif->ipaddr);
    ipaddr_set_any(&netif->netmask);
    ipaddr_set_any(&netif->gateway);
    netif->mtu = 0;                      // 默认为0，即不限制
    netif->type = NETIF_TYPE_NONE;
    nlist_node_init(&netif->node);

    plat_strncpy(netif->name, dev_name, NETIF_NAME_SIZE);
    netif->name[NETIF_NAME_SIZE - 1] = '\0';

    // 初始化接收队列
    net_err_t err = fixq_init(&netif->in_q, netif->in_q_buf, NETIF_INQ_SIZE, NLOCKER_THREAD);
    if (err < 0) {
        dbg_error(DBG_NETIF, "netif in_q init error, err: %d", err);
        return (netif_t *)0;
    }

    // 初始化发送队列
    err = fixq_init(&netif->out_q, netif->out_q_buf, NETIF_OUTQ_SIZE, NLOCKER_THREAD);
    if (err < 0) {
        dbg_error(DBG_NETIF, "netif out_q init error, err: %d", err);
        fixq_destroy(&netif->in_q);
        return (netif_t *)0;
    }

    netif->state = NETIF_OPENED;        // 切换为opened

    // 插入队列中
    nlist_insert_last(&netif_list, &netif->node);
    return netif;
}
