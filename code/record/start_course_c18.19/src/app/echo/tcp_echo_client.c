#include "tcp_echo_client.h"

#include <WinSock2.h>
//#include <arpa/inet.h>
#include "sys_plat.h"

int tcp_echo_client_start (const char * ip, int port) {
    plat_printf("tcp echo client, ip: %s, port: %d\n", ip, port);

    WSADATA wsdata;
    WSAStartup(MAKEWORD(2, 2), &wsdata);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        plat_printf("tcp echo client: open socket error");
        goto end;
    }

    // struct sockaddr
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    if (connect(s, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        plat_printf("connect error");
        goto end;
    }

    char buf[128];
    plat_printf(">>");
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (send(s, buf, plat_strlen(buf) - 1, 0) <= 0) {
            plat_printf("write error");
            goto end;
        }

        plat_memset(buf, 0, sizeof(buf));
        int len = recv(s, buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            plat_printf("read error");
            goto end;
        }

        plat_printf("%s\n", buf);
        plat_printf(">>");
    }

    closesocket(s);
    // close(s);
end:
    if (s >= 0) {
        closesocket(s);
    }
    return -1;
}
