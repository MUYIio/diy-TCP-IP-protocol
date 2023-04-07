/**
 * @file arp.h
 * @author lishutong (527676163@qq.com)
 * @brief ARP协议支持
 * @version 0.1
 * @date 2022-10-29
 * 
 * @copyright Copyright (c) 2022
 * 
 * IPv4协议使用的是IPv4地址，其目标是设计跨越不同类型物理网络的分组交换。
 * 而以太网使用的是mac地址，需要使用正确的mac地址才能保证对方接受到数据包。
 * 因此，使用ARP协议(RFC826)用于在IPv4地址和网络硬件地址进行转换。
 * 
 * ARP协议被设计成兼容多种不同类型的物理网络，但实际总是被用于IPv4和mac地址之间的转换
 * IPv6不使用ARP协议，而使用ICMPv6中的邻居发现协议。
 * 另外还有一种RARP协议，用于缺少磁盘的系统，现在几乎很少使用
 */
#include <string.h>
#include "arp.h"
#include "net_cfg.h"
#include "mblock.h"
#include "dbg.h"
#include "nlocker.h"

static arp_entry_t cache_tbl[ARP_CACHE_SIZE];    // arp缓存
static mblock_t cache_mblock;                // 空闲arp分配结构
static nlist_t cache_list;                   // 动态表

/**
 * @brief 初始化ARP缓存表及链表
 * ARP缓存用于维护每个接口中得到的IPv4地址到mac地址之间的转换关系
 */
static net_err_t cache_init(void) {
    nlist_init(&cache_list);
    
    // 建立缓存链
    plat_memset(cache_tbl, 0, sizeof(cache_tbl));
    net_err_t err = mblock_init(&cache_mblock, cache_tbl, sizeof(arp_entry_t), ARP_CACHE_SIZE, NLOCKER_NONE);
    if (err < 0) {
        return err;
    }

    return NET_ERR_OK;
}

/**
 * @brief ARP模块初始化
 */
net_err_t arp_init(void) {
    // ARP缓存初始化
    net_err_t err = cache_init();
    if (err < 0) {
        dbg_error(DBG_ARP, "arp cache init failed.");
        return err;
    }

    return NET_ERR_OK;
}
