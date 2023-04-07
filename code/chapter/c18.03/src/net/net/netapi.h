/**
 * @file netapi.h
 * @author lishutong(527676163@qq.com)
 * @brief 网络接口
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 * 警告！！！
 * 如果使用标准socket接口，则应用程序中只能包含该文件。如果不使用，则无限制。
 * 并且该头文件不能被协议栈内部的其它任何文件所包含
 */
#ifndef NETAPI_H
#define NETAPI_H

#include "sys.h"
#include "tools.h"

char* x_inet_ntoa(struct in_addr in);
uint32_t x_inet_addr(const char* str);
int x_inet_pton(int family, const char *strptr, void *addrptr);
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len);

#undef htons
#define htons(v)                x_htons(v)

#undef ntohs
#define ntohs(v)                x_ntohs(v)

#undef htonl
#define htonl(v)                x_htonl(v)

#undef ntohl
#define ntohl(v)                x_ntohl(v)

#define inet_ntoa(addr)             x_inet_ntoa(addr)
#define inet_addr(str)              x_inet_addr(str)

#define inet_pton(family, strptr, addrptr)          x_inet_pton(family, strptr, addrptr)
#define inet_ntop(family, addrptr, strptr, len)     x_inet_ntop(family, addrptr, strptr, len)

#endif // NETAPI_H
