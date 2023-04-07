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

    // 第一步先解析域名
    struct hostent hent, *result;
    char buf[512];
    int err;
    if (gethostbyname_r(dest, &hent, buf, sizeof(buf), &result, &err) < 0) {
        printf("resolve name %s failed\n", dest);
        return;
    }

    // in_addr_t ipaddr = inet_addr(dest);
    in_addr_t ipaddr = *(in_addr_t *)hent.h_addr_list[0];

    // 创建套接字
    int sk = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sk < 0) {
        printf("create socket error\n");
        return;
    }

    // 先构建地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET; //地址规格
    addr.sin_port = 0;          // 端口未用
    addr.sin_addr.s_addr = ipaddr;;

    inet_ntop(AF_INET, &addr.sin_addr.s_addr, buf, sizeof(buf));
    printf("try to ping %s [%s]\n", dest, buf);

    // bind和connect测试
//#define USE_CONNECT
#ifdef USE_CONNECT
    connect(sk, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in));
#endif 

   // 设置超时
    struct timeval tmo;
    tmo.tv_sec = 5;
    tmo.tv_usec = 0;
    setsockopt(sk, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmo, sizeof(tmo));

    // 扣除开头的时间部分值
    size -= sizeof(clock_t);

    // 填充一些数据，方便后续比较
    int fill_size = size > PING_BUFFER_SIZE ? PING_BUFFER_SIZE : size;
    for (int i = 0; i < fill_size; i++) {
        ping->req.buf[i] = i;
    }

    // 遍历多次发送，每次序号在不断增加
    int total_size = sizeof(icmp_hdr_t) + sizeof(clock_t) + fill_size;
    for (int i = 0, seq = 0; i < count; i++, seq++) {
        // 设置echo/reply的包信息
        ping->req.echo_hdr.id = start_id;
        ping->req.echo_hdr.seq = seq;
        ping->req.echo_hdr.type = 8;        // 请求号码
        ping->req.echo_hdr.code = 0;
        ping->req.time = clock();        // 计算当前时间
        ping->req.echo_hdr.checksum = 0;
        ping->req.echo_hdr.checksum = checksum(&ping->req, total_size);

#ifdef USE_CONNECT
        ssize_t size = send(sk, (const char*)&ping->req, total_size, 0);
#else
        ssize_t size = sendto(sk, (const char *)&ping->req, total_size, 0,
                        (struct sockaddr *)&addr, sizeof(addr));
#endif

        if (size < 0) {
            printf("send ping request failed.\n");
            break;
        }

        // 接收数据, 注意先清空
        do {
            memset(&ping->reply, 0, sizeof(ping->reply));

#ifdef USE_CONNECT
            size = recv(sk, (char*)&ping->reply, sizeof(ping->reply), 0);
#else
            int addr_len = sizeof(addr);
            size = recvfrom(sk, (char*)&ping->reply, sizeof(ping->reply), 0, 
                        (struct sockaddr*)&addr, &addr_len);
#endif
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

        if (size > 0) {
            // 比较接收的数据部分，不同说明包坏了，跳过继续发ping()
            int recv_size = size - sizeof(ip_hdr_t) - sizeof(icmp_hdr_t);
            if (memcmp(ping->req.buf, ping->reply.buf, recv_size - sizeof(clock_t))) {
                printf("recv data error\n");
                continue;
            }

            // 显示响应信息
            // 收到的数据量有时会比发送的要少(大批量发数据到 ping 8.8.8.8)
            ip_hdr_t* iphdr = &ping->reply.iphdr;
            int send_size = fill_size + sizeof(clock_t);
            if (recv_size == send_size) {
                printf("Reply from %s: bytes = %d", inet_ntoa(addr.sin_addr), send_size);
            } else {
                printf("Reply from %s: bytes = %d(send = %d)", inet_ntoa(addr.sin_addr), recv_size, send_size);
            }

            // 时间差值
            int diff_ms = (clock() - ping->req.time) / (CLOCKS_PER_SEC / 1000);
            if (diff_ms < 1) {
                printf(" time<1ms, TTL=%d\n", iphdr->ttl);
            } else {
                printf(" time=%dms, TTL=%d\n", diff_ms, iphdr->ttl);
            }

            // 隔一小段时间再继续下一次请求
            sys_sleep(PING_INTERVAL_MS);
        }
    }

    // 不要忘了关闭
    close(sk);
}

