/**
 * @file socket.h
 * @author lishutong(527676163@qq.com)
 * @brief 套接字接口类型
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <stdint.h>
#include "tools.h"

// 取消系统内置的定义(如果有的话)，使用自己的
#undef AF_INET
#define AF_INET                 0               // IPv4协议簇

#undef SOCK_RAW
#define SOCK_RAW                1               // 原始数据报

#undef IPPROTO_ICMP
#define IPPROTO_ICMP            1               // ICMP协议

#undef INADDR_ANY
#define INADDR_ANY              0               // 任意IP地址，即全0的地址

#pragma pack(1)

/**
 * @brief IPv4地址
 */
struct x_in_addr {
    union {
        struct {
            uint8_t addr0;
            uint8_t addr1;
            uint8_t addr2;
            uint8_t addr3;
        };

        uint8_t addr_array[IPV4_ADDR_SIZE];

        #undef s_addr       // 在win上，s_addr好像是个宏，这里取消掉
        uint32_t s_addr;
    };
};


/**
 * @brief Internet地址，含端口和地址
 * 该结构在大小上与x_sockaddr相同，用于存储ipv4的地址和端口
 */
struct x_sockaddr_in {
    uint8_t sin_len;                // 整个结构的长度，值固定为16
    uint8_t sin_family;             // 地址簇：AF_INET
    uint16_t sin_port;              // 端口号
    struct x_in_addr sin_addr;      // IP地址
    char sin_zero[8];               // 填充字节
};
#pragma pack()

// socket API实现函数
int x_socket(int family, int type, int protocol);

#endif // NET_SOCKET_H

