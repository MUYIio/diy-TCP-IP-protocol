/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <string.h>
#include "lib_syscall.h"
#include "main.h"
#include <getopt.h>
#include <stdlib.h>
#include <sys/file.h>
//#include "fs/file.h"
#include "tftp_client.h"

// tftp -w -p 1233 -b 512/1024 ip filename
// static int tftp_open (const char * ip, uint16_t port, int blksize)
int main (int argc, char ** argv) {
    if (argc < 2) {
        puts("tftp get file from remote");
        puts("Usage: tftp [-p port] ip");
        return 0;
    }

    // https://www.cnblogs.com/yinghao-liu/p/7123622.html
    // optind是下一个要处理的元素在argv中的索引
    // 当没有选项时，变为argv第一个不是选项元素的索引。
    int port = TFTP_DEF_PORT;    // 缺省只打印一次
    char ch;
    while ((ch = getopt(argc, argv, "p:h")) != -1) {
        switch (ch) {
            case 'h':
                puts("tftp get file from remote");
                puts("Usage: tftp [-p port] [-b blksize] ip");
                return 0;
            case 'p':
                port = atoi(optarg);
                break;
            case '?':
                if (optarg) {
                    fprintf(stderr, "Unknown option: -%s\n", optarg);
                }
                return -1;
        }
    }

    // 参数错误
    if (optind >optind) {
        fprintf(stderr, "no ip \n");
        return -1;
    }
    char * ip = argv[optind];
    return tftp_start(ip, port);
}
