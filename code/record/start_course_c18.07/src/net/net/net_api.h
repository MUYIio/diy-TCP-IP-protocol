#ifndef NET_API_H
#define NET_API_H

#include "tools.h"
#include "socket.h"

char * x_inet_ntoa(struct x_in_addr in);
uint32_t x_inet_addr (const char * str);
int x_inet_pton(int family, const char *strptr, void *addrptr);
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len);

#undef htons
#define htons(x)        x_htons(v)

#undef x_ntohs
#define ntohs(x)        x_ntohs(v)

#undef x_htonl
#define htonl(x)        x_htonl(v)

#undef x_ntohl
#define ntohl(x)        x_ntohl(v)

#define inet_ntoa(in)   x_inet_ntoa(in)
#define inet_addr(str)    x_inet_addr(str)
#define inet_pton(faminly, strptr, addrptr)     x_inet_pton(faminly, strptr, addrptr)
#define x_inet_ntop(family, addrptr, strptr, len)   x_inet_ntop(family, addrptr, strptr, len)

#define sockaddr_in     x_sockaddr_in

#define socket(family, type, protocol)      x_socket(family, type, protocol)

#endif 