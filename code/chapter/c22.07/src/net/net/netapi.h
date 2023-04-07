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
#include "socket.h"

char* x_inet_ntoa(struct x_in_addr in);
uint32_t x_inet_addr(const char* str);
int x_inet_pton(int family, const char *strptr, void *addrptr);
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len);


// 以下使用了宏进行类型和函数的重命名
// 其作用有二：
// 一，为应用程序提供了标准的socket相关接口的和宏的定义
// 二, 避免类型冲突，即当应用程序使用标准socket接口函数和类型时，不会使用开发时宿主机自带的网络库中的实现
#define in_addr             x_in_addr
#define sockaddr_in         x_sockaddr_in
#define sockaddr            x_sockaddr
#define timeval         x_timeval

#define socket(family, type, protocol)              x_socket(family, type, protocol)
#define sendto(s, buf, len, flags, dest, dlen)      x_sendto(s, buf, len, flags, dest, dlen)
#define recvfrom(s, buf, len, flags, src, slen)     x_recvfrom(s, buf, len, flags, src, slen)
#define setsockopt(s, level, optname, optval, len)  x_setsockopt(s, level, optname, optval, len)
#define close(s)                                    x_close(s)
#define connect(s, addr, len)                       x_connect(s, addr, len)
#define bind(s, addr, len)                          x_bind(s, addr, len)
#define send(s, buf, len, flags)                    x_send(s, buf, len, flags)
#define recv(s, buf, len, flags)                    x_recv(s, buf, len, flags)

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
