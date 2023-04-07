/**
 * @file netif.h
 * @author lishutong(527676163@qq.com)
 * @brief 网络接口层代码
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 * 该接口层代码负责将所有的网络接口统一抽像为netif结构，并提供统一的接口进行处理
 */
#ifndef NETIF_H
#define NETIF_H

#include "ipaddr.h"
#include "fixq.h"
#include "net_cfg.h"
#include "pktbuf.h"

/**
 * @brief 硬件地址
 */
typedef struct _netif_hwaddr_t {
    uint8_t len;                            // 地址长度
    uint8_t addr[NETIF_HWADDR_SIZE];        // 地址空间
}netif_hwaddr_t;

/**
 * @brief 网络接口支持的操作
 */
struct _netif_t;       // 前置声明，避免编译出错
typedef struct _netif_ops_t {
    net_err_t(*open) (struct _netif_t* netif, void * data);
    void (*close) (struct _netif_t* netif);

    net_err_t (*xmit)(struct _netif_t* netif);
}netif_ops_t;

/**
 * @brief 网络接口类型
 */
typedef enum _netif_type_t {
    NETIF_TYPE_NONE = 0,                // 无类型网络接口
    NETIF_TYPE_ETHER,                   // 以太网
    NETIF_TYPE_LOOP,                    // 回环接口

    NETIF_TYPE_SIZE,
}netif_type_t;

/**
 * @brief 链路层处理接口
 */
typedef struct _link_layer_t {
    netif_type_t type;

    net_err_t (*open)(struct _netif_t* netif);
    void(*close)(struct _netif_t* netif);
    net_err_t (*in)(struct _netif_t* netif, pktbuf_t* buf);
    net_err_t (*out)(struct _netif_t* netif, ipaddr_t* dest, pktbuf_t* buf);
}link_layer_t;

/**
 * @brief 网络接口层代码
 */
typedef struct _netif_t {
    char name[NETIF_NAME_SIZE];             // 网络接口名字

    netif_hwaddr_t hwaddr;                  // 硬件地址
    ipaddr_t ipaddr;                        // ip地址
    ipaddr_t netmask;                       // 掩码
    ipaddr_t gateway;                       // 网关

    enum {                                  // 接口状态
        NETIF_CLOSED,                       // 已关注
        NETIF_OPENED,                       // 已经打开
        NETIF_ACTIVE,                       // 激活状态
    }state;

    netif_type_t type;                      // 网络接口类型
    int mtu;                                // 最大传输单元

    const netif_ops_t* ops;                 // 驱动类型
    void* ops_data;                         // 底层私有数据

    const link_layer_t* link_layer;         // 链路层结构

    nlist_node_t node;                       // 链接结点，用于多个链接网络接口
    
    fixq_t in_q;                            // 数据包输入队列
    void * in_q_buf[NETIF_INQ_SIZE];
    fixq_t out_q;                           // 数据包发送队列
    void * out_q_buf[NETIF_OUTQ_SIZE];

    // 可以在这里加入一些统计性的变量
}netif_t;

net_err_t netif_init(void);
netif_t* netif_open(const char* dev_name, const netif_ops_t* driver, void* driver_data);
net_err_t netif_set_addr(netif_t* netif, ipaddr_t* ip, ipaddr_t* netmask, ipaddr_t* gateway);
net_err_t netif_set_hwaddr(netif_t* netif, const uint8_t* hwaddr, int len);
net_err_t netif_set_active(netif_t* netif);
net_err_t netif_set_deactive(netif_t* netif);
void netif_set_default (netif_t * netif);
netif_t * netif_get_default (void);
net_err_t netif_close(netif_t* netif);
net_err_t netif_register_layer(int type, const link_layer_t* layer);

// 数据包输入输出管理
net_err_t netif_put_in(netif_t* netif, pktbuf_t* buf, int tmo);
net_err_t netif_put_out(netif_t * netif, pktbuf_t * buf, int tmo);
pktbuf_t* netif_get_in(netif_t* netif, int tmo);
pktbuf_t* netif_get_out(netif_t * netif, int tmo);
net_err_t netif_out(netif_t* netif, ipaddr_t* ipaddr, pktbuf_t* buf);

#endif  // NETIF_H

