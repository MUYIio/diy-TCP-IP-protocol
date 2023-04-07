/**
 * @file udp_echo_client.c
 * @author lishutong (527676163@qq.com)
 * @brief UDP回显示客户端
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022
 * 
 * 启动后，进入命令行模式，将键盘的字符串发给服务器然后等待服务器的连接
 */
#include <string.h>
#include <stdio.h>
#include "netapi.h"

/**
 * @brief udp回显客户端程序
 */
int udp_echo_client_start(const char* ip, int port) {
    printf("udp echo client, ip: %s, port: %d\n", ip, port);
    printf("Enter quit to exit\n");

    // 创建套接字，使用流式传输，即tcp
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        printf("open socket error");
        goto end;
    }

    // 连接的服务地址和端口
    struct x_sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = x_inet_addr(ip);   // ip地址，注意大小端转换
    server_addr.sin_port = x_htons(port);             // 取端口号，注意大小端转换

//#define USE_CONNECT         // 是否使用connect
#ifdef USE_CONNECT
    x_connect(s, (const struct x_sockaddr*)&server_addr, sizeof(server_addr));
#endif
    // 循环，读取一行后发出去
    printf(">>");
    char buf[128];
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 如果接收到quit，则退出断开这个过程
        if (strncmp(buf, "quit", 4) == 0) {
            break;
        }
 
        // 将数据写到服务器中，不含结束符
        // 在第一次发送前会自动绑定到本地的某个端口和地址中
        size_t total_len = strlen(buf);
#ifdef USE_CONNECT
        ssize_t size = x_send(s, buf, total_len, 0);
#else
        ssize_t size = sendto(s, buf, total_len, 0, (struct x_sockaddr *)&server_addr, sizeof(server_addr));
#endif
        if (size < 0) {
            printf("send error");
            goto end;
        }

        // 读取回显结果并显示到屏幕上，不含结束符
        memset(buf, 0, sizeof(buf));
#ifdef USE_CONNECT
        size = recv(s, buf, sizeof(buf), 0);
#else
        struct x_sockaddr_in remote_addr;
        x_socklen_t addr_len;
        size = recvfrom(s, buf, sizeof(buf), 0, (struct x_sockaddr*)&remote_addr, &addr_len);
#endif
        if (size < 0) {
            printf("send error");
            goto end;
        }
        buf[sizeof(buf) - 1] = '\0';        // 谨慎起见，写结束符

        // 在屏幕上显示出来
        printf("%s", buf);
        printf(">>");
    }

    // 关闭连接
end:
    if (s >= 0) {
        close(s);
    }
    return -1;
}

