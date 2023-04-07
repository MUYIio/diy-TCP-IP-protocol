/**
 * @file netapi.c
 * @author lishutong(527676163@qq.com)
 * @brief 网络接口
 * @version 0.1
 * @date 2022-11-09
 * 
 * @copyright Copyright (c) 2022
 * 
 * 网络编程接口定义
 */
#include "netapi.h"
#include "tools.h"

#define IPV4_STR_SIZE           16          // IPv4地址的字符串长

/**
 * @brief IP地址转换为字符串
 * 之所以放在这里，是使得调用方都有自己的副本，不会因内部的静态数据而导致相应冲突
 */
char* x_inet_ntoa(struct x_in_addr in) {
    static char buf[IPV4_STR_SIZE];
    plat_sprintf(buf, "%d.%d.%d.%d", in.addr0, in.addr1, in.addr2, in.addr3);
    return buf;
}

/**
 * @brief 将ip地址转换为32位的无符号数
 */
uint32_t x_inet_addr(const char* str) {
    if (!str) {
        return INADDR_ANY;
    }

    ipaddr_t ipaddr;
    ipaddr_from_str(&ipaddr, str);
    return ipaddr.q_addr;
}

/**
 * @brief 将点分十进制的IP地址转换为网络地址
 * 若成功则为1，若输入不是有效的表达式则为0，若出错则为-1
 */
int x_inet_pton(int family, const char *strptr, void *addrptr) {
    // 仅支持IPv4地址类型
    if ((family != AF_INET) || !strptr || !addrptr) {
        return -1;
    }
    struct x_in_addr * addr = (struct x_in_addr *)addrptr;

    ipaddr_t dest;
    ipaddr_from_str(&dest, strptr);
    addr->s_addr = dest.q_addr;
    return 1;
}

/**
 * @brief 将网络地址转换为字符串形式的地址
 */
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len) {
    if ((family != AF_INET) || !addrptr || !strptr || !len) {
        return (const char *)0;
    }

    struct x_in_addr * addr = (struct x_in_addr *)addrptr;
    char buf[IPV4_STR_SIZE];
    plat_sprintf(buf, "%d.%d.%d.%d", addr->addr0, addr->addr1, addr->addr2, addr->addr3);
    plat_strncpy(strptr, buf, len - 1);
    strptr[len - 1] = '\0';
    return strptr;
}
