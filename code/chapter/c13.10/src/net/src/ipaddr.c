﻿/**
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

/**
 * @brief 将str字符串转移为ipaddr_t格式
 */
net_err_t ipaddr_from_str(ipaddr_t * dest, const char* str) {
    // 必要的参数检查
    if (!dest || !str) {
        return NET_ERR_PARAM;
    }

    // 初始值清空
    dest->q_addr = 0;

    // 不断扫描各字节串，直到字符串结束
    char c;
    uint8_t * p = dest->a_addr;
    uint8_t sub_addr = 0;
    while ((c = *str++) != '\0') {
        // 如果是数字字符，转换成数字并合并进去
        if ((c >= '0') && (c <= '9')) {
            // 数字字符转换为数字，再并入计算
            sub_addr = sub_addr * 10 + c - '0';
        } else if (c == '.') {
            // . 分隔符，进入下一个地址符，并重新计算
            *p++ = sub_addr;
            sub_addr = 0;
        } else {
            // 格式错误，包含非数字和.字符
            return NET_ERR_PARAM;
        }
    }

    // 写入最后计算的值
    *p++ = sub_addr;
    return NET_ERR_OK;
}

/**
 * 获取缺省地址
 */
ipaddr_t * ipaddr_get_any(void) {
    static ipaddr_t ipaddr_any = { .q_addr = 0 };
    return &ipaddr_any;
}

/**
 * @brief 复制ip地址
 */
void ipaddr_copy(ipaddr_t * dest, const ipaddr_t * src) {
    if (!dest || !src) {
        return;
    }

    dest->q_addr = src->q_addr;
}

/**
 * 判断IP地址是否完成相同
 */
int ipaddr_is_equal(const ipaddr_t * ipaddr1, const ipaddr_t * ipaddr2) {
    return ipaddr1->q_addr == ipaddr2->q_addr;
}

/**
 * 将ip地址转换到缓冲区中
 * @param ip_buf ip缓冲区
 * @param src 源ip地址
 */
void ipaddr_to_buf(const ipaddr_t* src, uint8_t* ip_buf) {
    ip_buf[0] = src->a_addr[0];
    ip_buf[1] = src->a_addr[1];
    ip_buf[2] = src->a_addr[2];
    ip_buf[3] = src->a_addr[3];
}

/**
 * 复制ip地址，从指定字节内存中读取
 * @param dest 目标ip地址
 * @param ip_buf 源ip地按类似192, 168, 1, 1的字节顺序排放
 */
void ipaddr_from_buf(ipaddr_t* dest, const uint8_t * ip_buf) {
    dest->a_addr[0] = ip_buf[0];
    dest->a_addr[1] = ip_buf[1];
    dest->a_addr[2] = ip_buf[2];
    dest->a_addr[3] = ip_buf[3];
}
