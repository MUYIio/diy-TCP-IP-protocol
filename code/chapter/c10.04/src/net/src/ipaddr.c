/**
 * @file ipaddr.c
 * @author lishutong (527676163@qq.com)
 * @brief IP地址定义及接口函数
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 */
#include "ipaddr.h"

/**
 * 设置ip为任意ip地址
 * @param ip 待设置的ip地址
 */
void ipaddr_set_any(ipaddr_t * ip) {
    ip->q_addr = 0;
}
