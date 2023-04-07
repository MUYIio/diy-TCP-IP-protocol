/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>

int main (int argc, char ** argv) {
    *(char *)0 = 0x1234;

    int a = 3 / 0;
    return 0;
}
