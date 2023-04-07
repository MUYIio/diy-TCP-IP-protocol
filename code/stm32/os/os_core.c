#include "os_core.h"
#include "os_task.h"
#include "os_list.h"
#include "os_event.h"
#include "os_cfg.h"
#include "os_plat.h"
#include "os_mem.h"
#include "os_timer.h"


static os_core_t  os_core;


/**
 * 检查位图的正确性
*/
#if OS_DEBUG_ENABLE
void os_bitmap_check (os_bitmap_t * bitmap) {
    // 不计算idle所在的最低优先级队列
    for (int i = 0; i < OS_PRIO_CNT - 1; i++) {
        // 检查位的设置
        int group = i >> 3;
        int group_map = bitmap->map[group];
        if (bitmap->group & (1 << group)) {
            os_assert(group_map != 0);
        } else {
            // 没有任务在里面
            os_assert(group_map == 0);
        }

        os_list_t * list = &os_core.task_list[i];
        if (group_map & (1 << (i & 0x7))) {
            os_assert(os_list_first(list) != OS_NULL);
        } else {
            os_assert(os_list_first(list) == OS_NULL);
        }
    }        

    // 最低优先级必须总是置1
    os_assert(bitmap->map[(OS_PRIO_CNT - 1) >> 3] != 0);
}
#else
#define os_bitmap_check(bitmap)
#endif

/**
 * 初始化就绪位置
 * @param bitmap 位图
 */
void os_bitmap_init (os_bitmap_t * bitmap) {
    os_assert(bitmap != OS_NULL);

    bitmap->group = 0;
    for (int i = 0; i < sizeof(bitmap->map); i++) {
        bitmap->map[i] = 0;
    }
}

/**
 * 返回当前任务
 */
os_task_t * os_task_self (void) {
    return os_core.curr_task;
}

/**
 * 返回系统启动后的时钟周期数
 */
int os_tick_count(void) {
    os_protect_t protect = os_enter_protect();
    int ticks = os_core.ticks;
    os_leave_protect(protect);

    return ticks;
}

/**
 * 禁止任务调度
 */
void os_sched_disable (void) {
    os_protect_t protect = os_enter_protect();

    if (os_core.locked < 255) {
        os_core.locked++;
    }

    os_leave_protect(protect);
}

static os_task_t * get_next_task (void) {
	static const int8_t map_table[] = {
	    /* 00 */ -1,   0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};

    int group = map_table[os_core.task_map.group];           // 取就绪组
    int prio = (group << 3) + map_table[os_core.task_map.map[group]];  //取就绪位

    os_task_t * to = os_list_first(os_core.task_list + prio);
    return (to != os_core.curr_task) ? to : OS_NULL;
}

/**
 * 运行一次任务调试器，做一次任务切换
 */
void os_sched_run (void) {   
    os_protect_t protect = os_enter_protect();

    // 已经上锁，禁止调度
    if (os_core.locked > 0) {
        os_leave_protect(protect);
        return;
    }

    // 取最高优先级任务
    // 也许别的任务误改了，所以这里即便没改，也检查下
    os_bitmap_check(&os_core.task_map);

    os_task_t * to = get_next_task();
    if (to != os_core.curr_task){
        os_assert(to != OS_NULL);
        os_dbg("switch from %s to %s\n", os_core.curr_task->name, to->name);
        os_assert(((cpu_stack_t *)to->ctx > to->start_stack) && ((cpu_stack_t *)to->ctx < to->start_stack + to->stack_size));

        // 不能在这里加个。因为可能还没切换后就又发生了中断，然后又进入了sched
        // 然后又执行上面的代码，而此时from已经改变了，但实际未改变，进而导致压栈失败
        os_core.next_task = to;  
        os_task_switch(os_core.curr_task, os_core.next_task);   
    }

    os_leave_protect(protect);
}

/**
 * 释放CPU给同一优先级的其它任务
 */
void os_task_yield (void) {
    os_protect_t protect = os_enter_protect();

    os_task_t * self = os_task_self();
    os_list_t * list = &os_core.task_list[self->prio];
    if (os_list_last(list) != self) {
        // 有后续结点，将自己移到表尾，这样后一个就前面来了
        os_list_remove_head(list);
        os_list_append(list, self);

        // 然后切换过去
        os_core.curr_task = os_list_first(list);
        os_task_switch(self, os_core.curr_task);
    }
    os_leave_protect(protect); 
}

/**
 * 允许任务调度
 */
void os_sched_enable (void) {
    os_protect_t protect = os_enter_protect();

    if (os_core.locked > 0) {
        if (--os_core.locked == 0) {
            // 可能有更高优先级的任务就绪，因此进行一次调度
            os_sched_run(); 
        }
    }

    os_leave_protect(protect);
}

/**
 * 将任务设置为就绪状态
 */
void os_sched_set_ready (os_task_t * task) {
    os_assert(task != OS_NULL);
    os_assert((task->prio >= 0) && (task->prio < OS_PRIO_CNT));

    os_list_append(os_core.task_list + task->prio, task);

    int group = task->prio >> 3;
    int bit = task->prio & 0x7;
    os_core.task_map.group |= 1 << group;
    os_core.task_map.map[group] |= 1 << bit;

    os_bitmap_check(&os_core.task_map);
}

/**
 * 将任务从就绪列表中移除
 */
void os_sched_set_unready (os_task_t * task) {
    os_assert(task != OS_NULL);
    os_assert((task->prio >= 0) && (task->prio < OS_PRIO_CNT));

    os_list_t * list = os_core.task_list + task->prio;

    os_list_remove(list, task);
    if (os_list_first(list) == OS_NULL) {
       int group = task->prio >> 3;
        int bit = task->prio & 0x7;

        os_core.task_map.map[group] &= ~(1 << bit);
        if (os_core.task_map.map[group] == 0) {
            os_core.task_map.group &= ~(1 << group);
        }

        os_bitmap_check(&os_core.task_map);
    }
}

/**
 * 设置进入延时状态
 */
void os_task_set_delay (os_task_t * task, int ms) {
    if (ms == 0) {
        return;
    }

    int ticks = (ms + OS_SYSTICK_MS - 1) / OS_SYSTICK_MS;
    if (ticks <= 0) {
        ticks = 1;
    }
    task->delay_tick = ticks;
    task->flags |= OS_TASK_FLAG_DELAY;

    for (os_task_t * curr = os_list_first(&os_core.delay_list); curr; curr = curr->next) {
        if (curr->delay_tick < task->delay_tick) {
            task->delay_tick -= curr->delay_tick;
            continue;
        } else if (curr->delay_tick == task->delay_tick) {
            task->delay_tick = 0;
            os_list_insert_after(&os_core.delay_list, curr, task);
            os_list_show(&os_core.delay_list, "delay list");
            return;
        } else {
            curr->delay_tick -= task->delay_tick;
            os_list_insert_after(&os_core.delay_list, curr->pre, task);
            os_list_show(&os_core.delay_list, "delay list");
            return;
        }
    }

    os_list_append(&os_core.delay_list, task); 
    os_list_show(&os_core.delay_list, "delay list");
}

/**
 * 将延时的任务从延时队列中唤醒
 */
void os_task_set_wakeup (os_task_t * task) {
    os_list_remove(&os_core.delay_list, task);
    task->flags &= ~OS_TASK_FLAG_DELAY;
}

/**
 * 系统时钟节拍处理
 */
void os_time_tick (void) {    
    os_protect_t protect = os_enter_protect();
    
    // 处理定时到的情况，保需要检查第一个非0的表项和后面全为0的表项
    os_task_t * task = os_list_first(&os_core.delay_list);
    while (task && (task->delay_tick-- <= 0)) {
        os_task_t * next = task->next;

        // 唤醒任务，通知超时
        task->delay_tick = 0;
        if (task->wait) {
            os_event_wakeup_task(task->wait->event, task, OS_NULL, OS_ERR_TMO);
        } else {
            os_task_set_wakeup(task);
            os_sched_set_ready(task);
        }

        // 取下一个任务
        task = next;
    }

    // 检查时间片
    if (--os_core.curr_task->slice <= 0) {
        os_core.curr_task->slice = OS_TASK_SLICE;        // 重置计数

        // 如果有后续任务，切换到后面的任务，后一个任务再上前
        if (os_core.curr_task->next) {          
            os_list_t * task_list = os_core.task_list + os_core.curr_task->prio;  
            os_list_remove_head(task_list);
            os_list_append(task_list, os_core.curr_task);
        }
    }

    // 节拍计数增加
    os_core.ticks++;

    os_leave_protect(protect);
}

void os_task_insert_delete(os_task_t * task) {
    os_list_append(&os_core.delete_list, task);
}

/**
 * 保存当前栈的指针并加载新栈的指针
 */
os_task_ctx_t *os_switch_ctx(os_task_ctx_t *ctx) {
    if (ctx != (os_task_ctx_t *)0) {
        os_core.curr_task->ctx = ctx;
        os_assert((uint8_t *)os_core.curr_task->ctx > (uint8_t *)os_core.curr_task->start_stack);
        os_assert((uint8_t *)os_core.curr_task->ctx < (uint8_t *)os_core.curr_task->start_stack + os_core.curr_task->stack_size);
    }

    os_core.curr_task = os_core.next_task;
    return os_core.next_task->ctx;
}

// 空闲任务的相关处理
static cpu_stack_t idle_stack[OS_IDLE_STACK_SIZE / sizeof(cpu_stack_t)];
static os_task_t idle_task;

/**
 * 空闲任务函数
 */
static void idle_task_entry (void * param) {
    for (;;) {
        os_protect_t protect = os_enter_protect();
        os_task_t * task = os_list_remove_head(&os_core.delete_list);
        os_core.task_cnt--;
        os_leave_protect(protect);

        if (task) {
            os_mem_free(task->start_stack);
            os_mem_free(task);
        }
    }
}

/**
 * 初始化内核数据结构
*/
os_err_t os_init (void) {
    os_plat_init();
    os_mem_init();

    os_core.curr_task = os_core.next_task = OS_NULL;
    os_core.locked = 0;      //  初始锁计数为0

    // 就绪队列设置
    os_bitmap_init(&os_core.task_map);
    for (int i = 0; i < OS_PRIO_CNT; i++) {
        os_list_init(&os_core.task_list[i]);
    }
    os_list_init(&os_core.delete_list);

    // 时间相关
    os_core.ticks = 0;
    os_list_init(&os_core.delay_list);
    os_timer_core_init();

    // 创建空闲任务
    os_err_t err = os_task_init(&idle_task, 
                "idle", 
                idle_task_entry, OS_NULL, OS_PRIO_CNT - 1, 
                idle_stack, OS_IDLE_STACK_SIZE);
    if (err < 0) {
        os_dbg("create idle task failed.");
        return err;
    }
    os_task_start(&os_core.idle_task);
    return OS_ERR_OK;
}

/**
 * 启动操作系统
 */
void os_start (void) {
    os_core.next_task = get_next_task();
    os_task_switch_to(os_core.next_task);
    for (;;) {}
}

void task_insert_os(os_task_t * task) {
    os_core.task_cnt++;
}

