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
#include "sock.h"

// 取消系统内置的定义(如果有的话)，使用自己的
#undef AF_INET
#define AF_INET                 0               // IPv4协议簇

#undef SOCK_RAW
#define SOCK_RAW                1               // 原始数据报
#undef SOCK_DGRAM
#define SOCK_DGRAM              2               // 数据报式套接字
#undef SOCK_STREAM
#define SOCK_STREAM             3               // 流式套接字

#undef IPPROTO_ICMP
#define IPPROTO_ICMP            1               // ICMP协议
#undef IPPROTO_UDP
#define IPPROTO_UDP             17              // UDP协议

#undef IPPROTO_TCP
#define IPPROTO_TCP             6               // TCP协议

#undef INADDR_ANY
#define INADDR_ANY              0               // 任意IP地址，即全0的地址

#undef SOL_SOCKET
#define SOL_SOCKET              0
#undef SOL_TCP
#define SOL_TCP                 1

// sock选项
#undef SO_RCVTIMEO
#define SO_RCVTIMEO             1            // 设置用于阻止接收调用的超时（以毫秒为单位）
#undef SO_SNDTIMEO
#define SO_SNDTIMEO             2            // 用于阻止发送调用的超时（以毫秒为单位）。
#undef SO_KEEPALIVE
#define SO_KEEPALIVE            3            // Keepalive选项
#undef TCP_KEEPIDLE
#define TCP_KEEPIDLE            4           // Keepalive空闲时间
#undef TCP_KEEPINTVL                        
#define TCP_KEEPINTVL           5           // 超时间隔
#undef TCP_KEEPCNT
#define TCP_KEEPCNT             6           // 重试的次数

#pragma pack(1)

typedef uint32_t x_in_addr_t;

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
 * @brief 通用socket结构，与具体的协议簇无关
 */
struct x_sockaddr {
    uint8_t sa_len;                 // 整个结构的长度，值固定为16
    uint8_t sa_family;              // 地址簇：NET_AF_INET
    uint8_t sa_data[14];            // 数据空间
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

/**
 * @brief 时间结构
 */
struct x_timeval {
    int tv_sec;             // 秒
    int tv_usec;            // 微秒
};

/**
 * @brief 套接字结构
 */
typedef struct _socket_t {
    enum {
        SOCKET_STATE_FREE,
        SOCKET_STATE_USED,
    }state;

    sock_t * sock;                  // 操作的sock结构
}x_socket_t;

// socket API实现函数
int x_socket(int family, int type, int protocol);
ssize_t x_sendto(int sid, const void* buf, size_t len, int flags, const struct x_sockaddr* dest, x_socklen_t dest_len);
ssize_t x_recvfrom(int sid, void* buf, size_t len, int flags, struct x_sockaddr* src, x_socklen_t* src_len);
int x_setsockopt(int sockfd, int level, int optname, const char * optval, int optlen);
int x_close(int sockfd);
int x_connect(int sid, const struct x_sockaddr* addr, x_socklen_t len);
int x_bind(int sid, const struct x_sockaddr* addr, x_socklen_t len);
ssize_t x_send(int fd, const void* buf, size_t len, int flags);
ssize_t x_recv(int fd, void* buf, size_t len, int flags);
int x_listen(int sockfd, int backlog);
int x_accept(int sockfd, struct x_sockaddr* addr, x_socklen_t* len);
ssize_t x_write(int sockfd, const void* buf, size_t len);
ssize_t x_read(int sockfd, void* buf, size_t len);

struct x_hostent {
         char *h_name;                      // 正式主机名
         char **h_aliases;                  // 别名列表指针数组，以NULL结束
         int h_addrtype;                    // 主机地址类型：AF_INET || AF_INET6
         int h_length;                      // 地址长度
         char **h_addr_list;                // 地址列表指针数组，以NULL结束
};
#define h_addr h_addr_list[0]      // for backward compatibility

/**
 * @brief hostent相关的附加参数
 * 只存储IP地址，不使用域名
 */
typedef struct _hostent_extra_t {
    x_in_addr_t * addr_tbl[2];
    x_in_addr_t addr;
    char name[1];
}hostent_extra_t;

int x_gethostbyname_r (const char *name,struct x_hostent *ret, char *buf, size_t buflen,
         struct x_hostent **result, int *h_errnop);

#endif // NET_SOCKET_H

