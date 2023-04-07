/**
 * 进程启动C部分代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"
#include <stdlib.h>

int main (int argc, char ** argv);

extern uint8_t __bss_start__[], __bss_end__[];

/**
 * @brief 应用的初始化，C部分
 */
void cstart (int argc, char ** argv) {
    // 清空bss区，注意这是必须的！！！
    // 像newlib库中有些代码就依赖于此，未清空时数据未知，导致调用sbrk时申请很大内存空间
    uint8_t * start = __bss_start__;
    while (start < __bss_end__) {
        *start++ = 0;
    }

    exit(main(argc, argv));
}