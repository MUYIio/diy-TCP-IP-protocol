#ifndef OS_TIMER_H
#define OS_TIMER_H

#include "os_cfg.h"
#include "os_event.h"

typedef void (*os_timer_func_t) (void * arg);

typedef enum _os_timer_state_t {
    OS_TIMER_INITED = 0,            // 已经初始化
    OS_TIMER_STARTED,               // 已经启动
    OS_TIMER_STOPPED,               // 启动后已经停止
}os_timer_state_t;

#define OS_TIMER_HD             (1 << 0)    // 硬定时器
#define OS_TIMER_ST             (0 << 0)    // 软定时器

/**
 * 定时器
 */
typedef struct _os_timer_t {
    struct _os_timer_t * next;  // 下一定时器
    int delay_ms;               // 初始启动延时时间
    int tmo_ms;                 // 超时时间
    int curr_ticks;             // 当前时间
    
    struct {
        int flags : 4;          // 配置标志
        int state : 4;          // 定时器状态
    };

    os_timer_func_t func;       // 定时回调函数
    void * arg;                 // 回调参数
}os_timer_t;

os_err_t os_timer_init (os_timer_t * timer, int delay_ms, int tmo_ms, os_timer_func_t func, void * arg, int flags);
os_err_t os_timer_uninit(os_timer_t * timer);
os_timer_t * os_timer_create (int delay_ms, int tmo_ms, os_timer_func_t func, void * arg, int flags);
os_err_t os_timer_free (os_timer_t * timer);
os_err_t os_timer_start (os_timer_t * timer);
os_err_t os_timer_stop (os_timer_t * timer);
int os_timer_left (os_timer_t * timer);

void os_timer_tick (void);
void os_timer_core_init (void);

#endif /* TTIMER_H */
