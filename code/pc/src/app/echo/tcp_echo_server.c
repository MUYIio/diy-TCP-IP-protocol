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
#include "netapi.h"
#include "sys_plat.h"
#include "tcp_echo_server.h"
static int server_port;

 /**
  * @brief TCP回显服务器程序
  */
static void tcp_echo_server_thread (void * arg) {
    plat_printf("tcp server start, port = %d\n", server_port);

    // 创建套接字，使用流式传输，即tcp
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        plat_printf("open socket error");
        return;
    }

    // 绑定到本地端口
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;   // ip地址，注意大小端转换
    server_addr.sin_port = htons(server_port);             // 取端口号，注意大小端转换
    if (bind(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("connect error");
        goto end;
    }

    listen(s, 5);
    while (1) {
        // 等待客户端的连接
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client = accept(s,  (struct sockaddr*)&client_addr, &addr_len);
        if (client < 0) {
            printf("accept error");
            break;
        }

        printf("tcp echo server:connect ip: %s, port: %d\n", x_inet_ntoa(client_addr.sin_addr), x_ntohs(client_addr.sin_port));

        // 循环，读取一行后发出去
        char buf[128];
        ssize_t size;
        while ((size = read(client, buf, sizeof(buf))) > 0) {
            printf("recv bytes: %d\n", (int)size);
            write(client, buf, size);
        }

        // 关闭连接
        close(client);
    }
end:
    close(s);
}

 /**
  * @brief TCP回显服务器程序
  */
int tcp_echo_server_start(int port) {
    server_port = port;

    // 创建单独的线程来运行服务
    sys_thread_t thread = sys_thread_create(tcp_echo_server_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        printf("tcp echo: create server thread failed\n");
		return -1;
    }
    return 0;
}

