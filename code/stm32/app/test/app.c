
#include "os_api.h"

#define OS_FIRST_STACK_SIZE		        1024		    // 初始任务的堆栈大小，以字节计

// 初始任务结构及栈空间
static os_task_t first_task;
static cpu_stack_t first_stack[OS_FIRST_STACK_SIZE / sizeof(cpu_stack_t)];

/**
 * 初始任务
 */
static void first_task_entry (void * arg) {
    os_app_init ();

    while (1) {
        os_app_loop();
    }
    
}

int main (void) {    
    // 初始化各种硬件平台，如存储、引脚、时钟、中断等，但注意全部中断使能关闭
    os_init();

    // 创建首个任务
    os_task_init(&first_task, "main thread", first_task_entry, OS_NULL, 0, 
                    (cpu_stack_t *)first_stack, OS_FIRST_STACK_SIZE);
    os_task_start(&first_task);
    
    // 启动多任务运行
    os_start();
    return 0;
}

