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
#include "ping.h"

int main (int argc, char ** argv) {
    // https://www.cnblogs.com/yinghao-liu/p/7123622.html
    // optind是下一个要处理的元素在argv中的索引
    // 当没有选项时，变为argv第一个不是选项元素的索引。
    int count = 4;    // 缺省只打印一次
    int interval = 1000;
    int size = 64;
    int ch;
    while ((ch = getopt(argc, argv, "n:t:l:h")) != -1) {
        switch (ch) {
            case 'h':
                puts("send ping to dest");
                puts("Usage: ping [-n count] [-t interval] [-l size] dest");
                optind = 1;        // getopt需要多次调用，需要重置
                return 0;
            case 'n':
                count = atoi(optarg);
                break;
            case 't':
                interval = atoi(optarg);
                break;
            case 'l':
                size = atoi(optarg);
                break;
            case '?':
                if (optarg) {
                    fprintf(stderr, "Unknown option: -%s\n", optarg);
                }
                optind = 1;        // getopt需要多次调用，需要重置
                return -1;
        }
    }

    // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
    if (optind > argc - 1) {
        fprintf(stderr, "Message is empty \n");
        optind = 1;        // getopt需要多次调用，需要重置
        return -1;
    }
    char * dest = argv[optind];

    // 调用ping
    ping_t ping;
    ping_run(&ping, dest, count, size, interval);
    optind = 1;        // getopt需要多次调用，需要重置
    return 0;
}
