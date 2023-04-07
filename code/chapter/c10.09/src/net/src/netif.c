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
#include "pktbuf.h"

static netif_t netif_buffer[NETIF_DEV_CNT];     // 网络接口的数量
static mblock_t netif_mblock;                   // 接口分配结构
static nlist_t netif_list;               // 网络接口列表
static netif_t * netif_default;          // 缺省的网络列表


/**
 * @brief 显示系统中的网卡列表信息
 */
#if DBG_DISP_ENABLED(DBG_NETIF)
void display_netif_list (void) {
    nlist_node_t * node;

    plat_printf("netif list:\n");
    nlist_for_each(node, &netif_list) {
        netif_t * netif = nlist_entry(node, netif_t, node);
        plat_printf("%s:", netif->name);
        switch (netif->state) {
            case NETIF_CLOSED:
                plat_printf(" %s ", "closed");
                break;
            case NETIF_OPENED:
                plat_printf(" %s ", "opened");
                break;
            case NETIF_ACTIVE:
                plat_printf(" %s ", "active");
                break;
            default:
                break;
        }
        switch (netif->type) {
            case NETIF_TYPE_ETHER:
                plat_printf(" %s ", "ether");
                break;
            case NETIF_TYPE_LOOP:
                plat_printf(" %s ", "loop");
                break;
            default:
                break;
        }
        plat_printf(" mtu=%d ", netif->mtu);
        plat_printf("\n");
        dump_mac("\tmac:", netif->hwaddr.addr);
        dump_ip_buf(" ip:", netif->ipaddr.a_addr);
        dump_ip_buf(" netmask:", netif->netmask.a_addr);
        dump_ip_buf(" gateway:", netif->gateway.a_addr);

        // 队列中包数量的显示
        plat_printf("\n");
    }
}
#else
#define display_netif_list()
#endif // DBG_NETIF

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
netif_t* netif_open(const char* dev_name, const netif_ops_t* ops, void * ops_data) {
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
    netif->ops = ops;                   // 设置驱动和私有参数
    netif->ops_data = (void *)ops_data;

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

    // 打开设备，对硬件做进一步的设置, 在其内部可对netif字段进行设备
    // 特别是要对type和link_layer做设备
    err = ops->open(netif, ops_data);
    if (err < 0) {
        dbg_error(DBG_NETIF, "netif ops open error: %d");
        goto free_return;
    }
    netif->state = NETIF_OPENED;        // 切换为opened

    // 驱动初始化完成后，对netif进行进一步检查
    // 做一些必要性的检查，以免驱动没写好
    if (netif->type == NETIF_TYPE_NONE) {
        dbg_error(DBG_NETIF, "netif type unknown");
        goto free_return;
    }

    // 插入队列中
    nlist_insert_last(&netif_list, &netif->node);
    display_netif_list();
    return netif;

free_return:
    if (netif->state == NETIF_OPENED) {
        netif->ops->close(netif);
    }

    fixq_destroy(&netif->in_q);
    fixq_destroy(&netif->out_q);
    mblock_free(&netif_mblock, netif);
    return (netif_t*)0;
}

/**
 * @brief 设置IP地址、掩码、网关等
 * 这里只是简单的设置接口的各个地址，进行写入
 */
net_err_t netif_set_addr(netif_t* netif, ipaddr_t* ip, ipaddr_t* netmask, ipaddr_t* gateway) {
    ipaddr_copy(&netif->ipaddr, ip ? ip : ipaddr_get_any());
    ipaddr_copy(&netif->netmask, netmask ? netmask : ipaddr_get_any());
    ipaddr_copy(&netif->gateway, gateway ? gateway : ipaddr_get_any());
    return NET_ERR_OK;
}

/**
 * @brief 设置硬件地址
 */
net_err_t netif_set_hwaddr(netif_t* netif, const uint8_t* hwaddr, int len) {
    plat_memcpy(netif->hwaddr.addr, hwaddr, len);
    netif->hwaddr.len = len;
    return NET_ERR_OK;
}

/**
 * @brief 激活网络设备
 */
net_err_t netif_set_active(netif_t* netif) {
    // 必须为打开状态地能激活
    if (netif->state != NETIF_OPENED) {
        dbg_error(DBG_NETIF, "netif is not opened");
        return NET_ERR_STATE;
    }

    // 看看是否要添加缺省接口
    // 缺省网络接口用于外网数据收发时的包处理
    if (!netif_default && (netif->type != NETIF_TYPE_LOOP)) {
        netif_set_default(netif);
    }

    // 切换为就绪状态
    netif->state = NETIF_ACTIVE;
    display_netif_list();
    return NET_ERR_OK;
}

/**
 * @brief 取消设备的激活态
 */
net_err_t netif_set_deactive(netif_t* netif) {
    // 必须已经激活的状态
    if (netif->state != NETIF_ACTIVE) {
        dbg_error(DBG_NETIF, "netif is not actived");
        return NET_ERR_STATE;
    }

    // 释放掉队列中的所有数据包
    pktbuf_t* buf;
    while ((buf = fixq_recv(&netif->in_q, -1))) {
        pktbuf_free(buf);
    }
    while ((buf = fixq_recv(&netif->out_q, -1))) {
        pktbuf_free(buf);
    }

    // 重设缺省网口
    if (netif_default == netif) {
        netif_default = (netif_t*)0;
    }

    // 切换回打开（非激活状态）
    netif->state = NETIF_OPENED;
    display_netif_list();
    return NET_ERR_OK;
}

/**
 * @brief 关闭网络接口
 */
net_err_t netif_close(netif_t* netif) {
    // 需要先取消active状态才能关闭
    if (netif->state == NETIF_ACTIVE) {
        dbg_error(DBG_NETIF, "netif(%s) is active, close failed.", netif->name);
        return NET_ERR_STATE;
    }

    // 先关闭设备
    netif->ops->close(netif);
    netif->state = NETIF_CLOSED;

    // 最后释放netif结构
    nlist_remove(&netif_list, &netif->node);
    mblock_free(&netif_mblock, netif);
    display_netif_list();
    return NET_ERR_OK;
}

/**
 * @brief 设置缺省的网络接口
 * @param netif 缺省的网络接口
 */
void netif_set_default(netif_t* netif) {
    // 纪录新的网卡
    netif_default = netif;
}
