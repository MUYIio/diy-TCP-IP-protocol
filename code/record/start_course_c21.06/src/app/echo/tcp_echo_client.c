#include "tcp_echo_client.h"

//#include <WinSock2.h>
//#include <arpa/inet.h>
#include "sys_plat.h"
#include "net_api.h"

int tcp_echo_client_start (const char * ip, int port) {
    plat_printf("tcp echo client, ip: %s, port: %d\n", ip, port);

    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
#if 1
    char sbuf[128];
    fgets(sbuf, sizeof(sbuf), stdin);
    close(s);
    return 0;
#endif

#if 0
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
#endif

    close(s);
end:
    if (s >= 0) {
        close(s);
    }
    return -1;
}
