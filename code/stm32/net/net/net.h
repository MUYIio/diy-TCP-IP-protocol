/**
 * @brief TCP/IP协议栈初始化
 *
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#ifndef NET_H
#define NET_H

#include "net_err.h"

net_err_t net_init (void);
net_err_t net_start(void);

#endif // NET_H
