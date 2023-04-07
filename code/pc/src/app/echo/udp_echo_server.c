/**
 * @brief 基于udp的echo服务器
 * 使用方法：程序名称 端口号
 * 使用udp echo客户端连接后，任何发过来的信息将原样回显
 * 
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 */
#include "netapi.h"

static uint16_t server_port;

/**
 * 服务器线程入口
 */
static void udp_echo_server(void * arg) {
    // 打开套接字
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        printf("open socket error\n");
        goto end;
    }

    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = x_htons(server_port);
    local_addr.sin_addr.s_addr = x_htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        printf("bind error\n");
        goto end;
    }

    // 绑定本地端口
    while (1) {
        struct x_sockaddr_in client_addr;
        char buf[256];

        // 接受来自客户端的数据包
        x_socklen_t addr_len = sizeof(client_addr);
        ssize_t size = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (size < 0) {
            printf("recv from error\n");
            goto end;
        }

        // 加上下面的打印之后，由于比较费时，会导致UDP包来不及接收被被丢弃
        plat_printf("udp echo server:connect ip: %s, port: %d\n", inet_ntoa(client_addr.sin_addr), x_ntohs(client_addr.sin_port));

        // 发回去
        size = sendto(s, buf, size, 0, (struct sockaddr *)&client_addr, addr_len);
        if (size < 0) {
            printf("sendto error\n");
            goto end;
        }
    }
end:
    if (s >= 0) {
        close(s);
    }
}

/**
 * @brief 启动UDP回显服务
 */
net_err_t udp_echo_server_start (int port) {
    printf("udp echo server, port: %d\n", port);

    server_port = port;
    if (sys_thread_create(udp_echo_server, (void *)0) == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }

    return NET_ERR_OK;
}
