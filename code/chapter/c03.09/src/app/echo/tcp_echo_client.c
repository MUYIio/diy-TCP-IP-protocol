/**
 * @brief 简单的tcp echo 客户端程序
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
#include "tcp_echo_client.h"
#include "sys_plat.h"

 /**
  * @brief TCP回显客户端程序
  */
int tcp_echo_client_start (const char* ip, int port) {
    plat_printf("tcp echo client, ip: %s, port: %d\n", ip, port);
    plat_printf("Enter quit to exit\n");

    // 创建套接字，使用流式传输，即tcp
#if defined(SYS_PLAT_WINDOWS)
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
#else
    int s = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (s < 0) {
        plat_printf("tcp echo client: open socket error");
        goto end;
    }

    // 创建套接字，使用流式传输，即tcp
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);   // ip地址，注意大小端转换
    server_addr.sin_port = htons(port);             // 取端口号，注意大小端转换
    if (connect(s, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("connect error");
        goto end;
    }

    // 循环，读取一行后发出去
    char buf[128];
    plat_printf(">>");
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 将数据写到服务器中，不含结束符
        if (send(s, buf, plat_strlen(buf) - 1, 0) <= 0) {
            plat_printf("write error");
            goto end;
        }

        // 读取回显结果并显示到屏幕上，不含结束符
        plat_memset(buf, 0, sizeof(buf));
        int len = recv(s, buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            plat_printf("read error");
            goto end;
        }
        buf[sizeof(buf) - 1] = '\0';        // 谨慎起见，写结束符

        // 在屏幕上显示出来
        plat_printf("%s", buf);
        plat_printf(">>");
    }
#if defined(SYS_PLAT_WINDOWS)
    closesocket(s);
#else
    close(s);
#endif 
    return 0;

end:
    if (s >= 0) {
#if defined(SYS_PLAT_WINDOWS)
    closesocket(s);
#else
    close(s);
#endif 
    }
    return -1;
}

