/**
 * @brief 简单的TCP回显服务器程序
 *
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note 该程序仅限在Mac和Linux编程上编译通过，Windows由于套接字接口不同编译会失败
 */
#include <string.h>
#if defined(SYS_PLAT_WINDOWS)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include "sys_plat.h"
#include "tcp_echo_server.h"

 /**
  * @brief TCP回显服务器程序
  */
void tcp_echo_server_start(int port) {
    plat_printf("tcp server start, port = %d\n", port);

    // 创建套接字，使用流式传输，即tcp
#if defined(SYS_PLAT_WINDOWS)
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
#else
    int s = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (s < 0) {
        plat_printf("open socket error");
        return;
    }

    // 绑定到本地端口
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;   // ip地址，注意大小端转换
    server_addr.sin_port = htons(port);             // 取端口号，注意大小端转换
    if (bind(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("connect error");
        goto end;
    }

    listen(s, 5);
    while (1) {
        // 等待客户端的连接
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
#if defined(SYS_PLAT_WINDOWS)
        SOCKET client = accept(s,  (struct sockaddr*)&client_addr, &addr_len);
#else
        int client = accept(s,  (struct sockaddr*)&client_addr, &addr_len);
#endif
        if (client < 0) {
            printf("accept error");
            break;
        }

        printf("tcp echo server:connect ip: %s, port: %d\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 循环，读取一行后发出去
        char buf[128];
        ssize_t size;
        while ((size = recv(client, buf, sizeof(buf), 0)) > 0) {
            printf("recv bytes: %d\n", (int)size);
            send(client, buf, size, 0);
        }

        // 关闭连接
#if defined(SYS_PLAT_WINDOWS)
        closesocket(s);
#else
        close(s);
#endif
    }
end:
#if defined(SYS_PLAT_WINDOWS)
    closesocket(s);
#else
    close(s);
#endif
}

