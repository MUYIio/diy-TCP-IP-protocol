/**
 * @file ntp.c
 * @author lishutong(527676163@qq.com)
 * @brief NTP时间客户端
 * @version 0.1
 * @date 2022-11-04
 * 
 * @copyright Copyright (c) 2022
 * 
 * 测试服务器列表请见： http://www.ntp.org.cn/pool
 */
#include "ntp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 从NTP服务器获取最新的时间
 */
struct tm * request_time (const char * server) {
    // 创建访问的套接字
    int sockfd = x_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("ntp: create socket error\n");
        return (struct tm *)0;
    }

    // 设置超时
    struct x_timeval tmo;
    tmo.tv_sec = NTP_REQ_TMO;
    tmo.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tmo, sizeof(tmo));

    // 连接到服务器
    struct x_sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NTP_SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(server);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("ntp: connect to ntp server error\n");
        return (struct tm *)0;
    }

    // 重试多次
    struct tm * t = (struct tm *)0;
    for (int i = 0; i < NTP_REQ_RETRY; i++) {
        // 发送时间请求
        ntp_pkt_t pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.LI_VN_Mode = (NTP_VERSION << 3) | (NTP_MODE << 0); 
        if (x_send(sockfd, &pkt, sizeof(ntp_pkt_t), 0) < 0) {
            printf("ntp: send to ntp server error\n");
            break;
        }
    
        // 读取响应数据
        ssize_t size = x_recv(sockfd, (char*)&pkt, sizeof(ntp_pkt_t), 0);
        if (size < 0) {
            // 超时，继续重试
            printf("ntp: timeout\n");
            continue;
        }

        // 可以在此对包进行一些检查
        // 进行大小转换后转成本地时间. 这里不计算精确的时间，所以直接使用发送时的时间
        pkt.trans_ts.seconds = x_ntohl(pkt.trans_ts.seconds);
        pkt.trans_ts.fraction = x_ntohl(pkt.trans_ts.fraction);
        time_t ts = pkt.trans_ts.seconds - NTP_TIME_1900_1970;
        t = localtime(&ts);
        break;
    }

    // 数据读取完毕，关闭套接字
    x_close(sockfd);
    return t;
}

/**
 * @brief 获取当前时间
 */
struct tm * ntp_time (void) {
    // 时间服务器列表
    const char * server_list[] = {
        //"203.107.6.22",         // 不存在的NTP，测试超时用
        "203.107.6.88",
        "120.25.115.20",
        "223.113.120.195",
    };

    // 不断尝试，获得时间即退出
    for (int i = 0; i < sizeof(server_list) / sizeof(const char *); i++) {
        printf("ntp: try to get time from %s\n", server_list[i]);

        struct tm * t = request_time(server_list[i]);
        if (t) {
            return t;
        }

        printf("ntp: failed\n");
    }

    // 全部找不到
    printf("ntp: get time failed, stop\n");
    return (struct tm *)0;
}


