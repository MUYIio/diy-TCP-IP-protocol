#include "udp_echo_client.h"

#include <WinSock2.h>
//#include <arpa/inet.h>
//#include "net_api.h"

#include "sys_plat.h"

int udp_echo_client_start (const char * ip, int port) {
    plat_printf("udp echo client, ip: %s, port: %d\n", ip, port);

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        plat_printf("udp echo client: open socket error");
        goto end;
    }

    // struct sockaddr
    struct sockaddr_in server_addr;
    plat_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    connect(s, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    char buf[128];
    plat_printf(">>");
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (strncmp(buf, "quit", 4) == 0) {
            break;
        }

        // send/recv
        if (send(s, buf, plat_strlen(buf) - 1, 0) <= 0) {
        //if (sendto(s, buf, plat_strlen(buf) - 1, 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) <= 0) {
            plat_printf("write error");
            goto end;
        }

        struct sockaddr_in remote_addr;
        int addr_len = sizeof(remote_addr);
        plat_memset(buf, 0, sizeof(buf));
        int len = recv(s, buf, sizeof(buf), 0);
        //int len = recvfrom(s, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&remote_addr, &addr_len);
        if (len <= 0) {
            plat_printf("read error");
            goto end;
        }

        buf[sizeof(buf) - 1] = '\0';
        plat_printf("%s\n", buf);
        plat_printf(">>");
    }

    //closesocket(s);
    close(s);
end:
    if (s >= 0) {
        //closesocket(s);
        close(s);
    }
    return -1;
}
