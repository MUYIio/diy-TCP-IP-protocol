/**
 * ping程序的实现
 */
#include <string.h>
#include "ping.h"
#include "sys_plat.h"
/**
 * 计算校验和
 */
static uint16_t checksum(void* buf, uint16_t len) {
    uint16_t* curr_buf = (uint16_t*)buf;
    uint32_t checksum = 0;

    while (len > 1) {
        checksum += *curr_buf++;
        len -= 2;
    }

    if (len > 0) {
        checksum += *(uint8_t*)curr_buf;
    }

    // 注意，这里要不断累加。不然结果在某些情况下计算不正确
    uint16_t high;
    while ((high = checksum >> 16) != 0) {
        checksum = high + (checksum & 0xffff);
    }

    return (uint16_t)~checksum;
}

/**
 * 对指定目标发起ping请求
 * 只是简单的发送ping并判断响应，并统计是否有丢失，最大最小的时间值
 */
void ping_run(ping_t * ping, const char* dest, int count, int size, int interval) {
    static uint16_t start_id = PING_DEFAULT_ID;     // 起始id, 每次发的时候都不一样
    char buf[512];

    // 创建套接字
#if defined(SYS_PLAT_WINDOWS)
    SOCKET sk = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
#else
    int sk = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
#endif
    if (sk < 0) {
        printf("create socket error\n");
        return;
    }

    // 先构建地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; //地址规格
    addr.sin_port = 0;          // 端口未用
    addr.sin_addr.s_addr = inet_addr(dest);;

    inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
    printf("try to ping %s [%s]\n", dest, buf);

    // 填充一些数据，方便后续比较
    int fill_size = size > PING_BUFFER_SIZE ? PING_BUFFER_SIZE : size;
    for (int i = 0; i < fill_size; i++) {
        ping->req.buf[i] = i;
    }

    // 遍历多次发送，每次序号在不断增加
    int total_size = sizeof(icmp_hdr_t) + fill_size;
    int total_size = sizeof(icmp_hdr_t) + fill_size;
    for (int i = 0, seq = 0; i < count; i++, seq++) {
        // 设置echo/reply的包信息
        ping->req.echo_hdr.id = start_id;
        ping->req.echo_hdr.seq = seq;
        ping->req.echo_hdr.type = 8;        // 请求号码
        ping->req.echo_hdr.code = 0;
        ping->req.echo_hdr.checksum = 0;
        ping->req.echo_hdr.checksum = checksum(&ping->req, total_size);

        ssize_t size = sendto(sk, (const char *)&ping->req, total_size, 0,
                        (struct sockaddr *)&addr, sizeof(addr));
        if (size < 0) {
            printf("send ping request failed.\n");
            break;
        }

        // 接收数据, 注意先清空
        do {
            memset(&ping->reply, 0, sizeof(ping->reply));

            int addr_len = sizeof(addr);
            size = recvfrom(sk, (char*)&ping->reply, sizeof(ping->reply), 0,
                        (struct sockaddr*)&addr, &addr_len);
            if (size < 0) {
                printf("ping recv tmo.\n");
                break;
            }

            // 序号要一样, 可以再考虑检查下地址
            if ((ping->req.echo_hdr.id == ping->reply.echo_hdr.id) &&
                (ping->req.echo_hdr.seq == ping->reply.echo_hdr.seq)) {
                break;
            }
        } while (1);

        printf("recv ping\n");
    }

    // 不要忘了关闭
#if defined(SYS_PLAT_WINDOWS)
    closesocket(sk);
#else
    close(sk);
#endif
}

