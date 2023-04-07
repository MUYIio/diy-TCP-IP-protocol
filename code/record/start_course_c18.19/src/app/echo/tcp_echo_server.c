#include "tcp_echo_server.h"

#include <WinSock2.h>
//#include <arpa/inet.h>
#include "sys_plat.h"

void tcp_echo_server_start (int port) {
    plat_printf("tcp server start, port = %d\n", port);

    WSADATA wsdata;
    WSAStartup(MAKEWORD(2, 2), &wsdata);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        plat_printf("tcp echo server: open socket error");
        goto end;
    }

    // struct sockaddr
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(s, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("bind error");
        goto end;
    }    

    listen(s, 5);
    while (1) {
        // 监听套接字
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        SOCKET client = accept(s, (struct sockaddr *)&client_addr, &addr_len);
        if (client < 0) {
            plat_printf("accept error");
            break;
        }

        plat_printf("tcp echo server: connect ip: %s, port: %d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
        );

        char buf[125];
        ssize_t size;
        while ((size = recv(client, buf, sizeof(buf), 0)) > 0) {
            plat_printf("recv size: %d\n", (int)size);
            send(client, buf, size, 0);
        }

        closesocket(client);
    }
end:
    if (s >= 0) {
        closesocket(s);
    }
}
