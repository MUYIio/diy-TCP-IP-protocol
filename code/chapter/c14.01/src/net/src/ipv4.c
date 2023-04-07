/**
 * @file ipv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "ipv4.h"
#include "dbg.h"

/**
 * @brief IP模块初始化
 */
net_err_t ipv4_init(void) {
    dbg_info(DBG_IP,"init ip\n");

    dbg_info(DBG_IP,"done.");
    return NET_ERR_OK;
}
