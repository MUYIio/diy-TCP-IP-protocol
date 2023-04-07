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
#ifndef ETHER_H
#define ETHER_H

#include "net_err.h"

net_err_t ether_init(void);

#endif // ETHER_H
