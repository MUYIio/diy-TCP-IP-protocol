/**
 * @file download.c
 * @author lishutong (527676163@qq.com)
 * @brief 简单的下载测试代码
 * @version 0.1
 * @date 2022-10-23
 *
 * @copyright Copyright (c) 2022
 * @note 该源码配套相应的视频课程，请见源码仓库下面的README.md
 */
#include <stdio.h>
#include <string.h>
#include "net_plat.h"
//#include <WinSock2.h>
#include "netapi.h"

static int port = 3000;

/**
 * @brief 简单的从对方客户机进行下载，以便测试TCP接收
 */
void download_test (const char * filename) {
    printf("try to download %s from %s: %d\n", filename, friend0_ip, port);

    // 创建服务器套接字，使用IPv4，数据流传输
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("create socket error\n");
		return;
	}

    // 绑定本地地址
    struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(friend0_ip);
    if (connect(sockfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("connect error\n");
        close(sockfd);
        return;
    }

    FILE * file = fopen(filename, "wb");
    if (file == (FILE *)0) {
        printf("open file failed.\n");
        goto failed;
    }

    char buf[8192];
    ssize_t total_size = 0;
    int rcv_size;
    while ((rcv_size = recv(sockfd, buf, sizeof(buf), 0)) > 0) {
        fwrite(buf, 1, rcv_size, file);
        printf(".");
        total_size += rcv_size;
    }
    if ((rcv_size == 0) || (rcv_size) == NET_ERR_EOF) {
        // 接收完毕
        fclose(file);
        close(sockfd);
        printf("rcv file size: %d\n", (int)total_size);
        return;
    }
failed:
    printf("rcv file error\n");
    close(sockfd);
    fclose(file);
    return;
}