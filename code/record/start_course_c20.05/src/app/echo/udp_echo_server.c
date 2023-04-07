#include "tcp_echo_server.h"

//#include <arpa/inet.h>
#include "sys_plat.h"
#include "net_api.h"

static uint16_t server_port;

void udp_echo_server (void * arg) {
    WSADATA wsdata;
    WSAStartup(MAKEWORD(2, 2), &wsdata);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        plat_printf("udp echo server: open socket error");
        goto end;
    }

    // struct sockaddr
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);
    //if (bind(s, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    //    plat_printf("bind error");
    //    goto end;
    //}    

    while (1) {
        struct sockaddr_in client_addr;
        char buf[125];
        int addr_len = sizeof(client_addr);
        ssize_t size = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (size < 0) {
            printf("recv error\n");
            goto end;
        }

        plat_printf("udp echo server: connect ip: %s, port: %d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)
        );

        sendto(s, buf, size, 0, (struct sockaddr *)&client_addr, addr_len);
        if (size < 0) {
            printf("sendto error\n");
            goto end;
        }
    }
end:
    if (s >= 0) {
        closesocket(s);
    }
}

void udp_echo_server_start (int port) {
    printf("udp echo server, port: %d\n", port);

    server_port = port;
    if (sys_thread_create(udp_echo_server, (void *)0) == SYS_THREAD_INVALID) {
        printf("create udp server thread failed.\n");
        return;
    }
}
