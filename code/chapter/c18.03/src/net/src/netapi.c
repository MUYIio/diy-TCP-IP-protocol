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

/**
 * @brief IP地址转换为字符串
 * 之所以放在这里，是使得调用方都有自己的副本，不会因内部的静态数据而导致相应冲突
 */
char* x_inet_ntoa(struct x_in_addr in) {
   return (char *)0;
}

/**
 * @brief 将ip地址转换为32位的无符号数
 */
uint32_t x_inet_addr(const char* str) {
    return 0;
}

/**
 * @brief 将点分十进制的IP地址转换为网络地址
 * 若成功则为1，若输入不是有效的表达式则为0，若出错则为-1
 */
int x_inet_pton(int family, const char *strptr, void *addrptr) {
    return 1;
}

/**
 * @brief 将网络地址转换为字符串形式的地址
 */
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len) {
    return (const char *)0;
}
