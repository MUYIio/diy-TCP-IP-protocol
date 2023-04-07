#include "net_api.h"

// 192.168.74.1
#define IPV4_STR_ADDR_SIZE      16

char * x_inet_ntoa(struct x_in_addr in) {
    static char buf[IPV4_STR_ADDR_SIZE];
    plat_sprintf(buf, "%d.%d.%d.%d", in.addr0, in.addr1, in.addr2, in.addr3);
    return buf;
}

uint32_t x_inet_addr (const char * str) {
    if (!str) {
        return INADDR_ANY;
    }

    ipaddr_t ipaddr;
    ipaddr_from_str(&ipaddr, str);
    return ipaddr.q_addr;
}

int x_inet_pton(int family, const char *strptr, void *addrptr) {
    if ((family != AF_INET) || !strptr || !addrptr) {
        return -1;
    }

    struct x_in_addr * addr = (struct x_in_addr *)addrptr;

    ipaddr_t ipaddr;
    ipaddr_from_str(&ipaddr, strptr);
    addr->s_addr = ipaddr.q_addr;
    return 0;
}

const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len) {
    if ((family != AF_INET) || !strptr || !addrptr || !len) {
        return (const char *)0;
    }

    struct x_in_addr * addr = (struct x_in_addr *)addrptr;
    char buf[IPV4_ADDR_SIZE];
    plat_sprintf(buf, "%d.%d.%d.%d", addr->addr0, addr->addr1, addr->addr2, addr->addr3);
    plat_strncpy(strptr, buf, len -1);
    strptr[len - 1] = '\0';
    return strptr;
}
