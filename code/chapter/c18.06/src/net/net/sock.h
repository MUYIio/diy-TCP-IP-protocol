/**
 * @file sock.h
 * @author lishutong(527676163@qq.com)
 * @brief 数据通信sock实现
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 * 提供基本的sock结构，用于实现RAW/TCP/UDP等接口的操作的底层支持
 */
#ifndef SOCK_H
#define SOCK_H

#include "exmsg.h"

net_err_t socket_init(void);

#endif // SOCK_H
