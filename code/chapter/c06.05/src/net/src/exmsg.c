/**
 * @brief TCP/IP核心线程通信模块
 * 此处运行了一个核心线程，所有TCP/IP中相关的事件都交由该线程处理
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#include "net_plat.h"
#include "exmsg.h"

/**
 * @brief 核心线程通信初始化
 */
net_err_t exmsg_init(void) {
    return NET_ERR_OK;
}

/**
 * @brief 核心线程功能体
 */
static void work_thread (void * arg) {
    // 注意要加上\n。否则由于C库缓存的关系，字符串会被暂时缓存而不输出显示
    plat_printf("exmsg is running....\n");

    while (1) {
        sys_sleep(1);
    }
}

/**
 * @brief 启动核心线程通信机制
 */
net_err_t exmsg_start(void) {
    // 创建核心线程
    sys_thread_t thread = sys_thread_create(work_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }

    return NET_ERR_OK;
}