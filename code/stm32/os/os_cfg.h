#ifndef OS_CFG_H
#define OS_CFG_H

#define	OS_PRIO_CNT				        64	            // OS任务的优先级最大序号
#define OS_TASK_SLICE				    10			    // 每个任务最大运行的时间片计数

#define OS_IDLE_STACK_SIZE		        512		        // 空闲任务的堆栈大小，以字节计
#define OS_FIRST_STACK_SIZE		        1024		    // 初始任务的堆栈大小，以字节计
#define OS_TIMERTASK_STACK_SIZE		    128			    // 定时器任务的堆栈大小，以字节计
#define OS_TIMERTASK_PRIO               1               // 定时器任务的优先级

#define OS_SYSTICK_MS                   10              // 时钟节拍的周期，以ms为单位
#define OS_CHECK_PARAM                  1               // 是否启用参数检查

#define OS_DEBUG_ENABLE                 1               // 是否开启调试输出


// 内核功能裁剪部分
#define OS_SEM_ENABLE                   1               // 是否使能信号量
#define OS_MUTEX_ENABLE                 1               // 是否使能互斥信号量
#define OS_ENABLE_MBLOCK                0               // 是否使能存储块

#define OS_DEBUG_ENABLE                 1               // 是否开启调试检查
#define OS_ENABLE_FLAGGROUP             0               // 是否使能事件标志组
#define OS_ENABLE_QUEUE                 0               // 是否使能邮箱
#define OS_ENABLE_MEMBLOCK              0               // 是否使能存储块
#define OS_ENABLE_TIMER                 0               // 是否使能定时器
#define OS_ENABLE_CPUUSAGE              0               // 是否使能CPU使用率统计
#define OS_ENABLE_HOOKS                 0               // 是否使能Hooks

#endif
