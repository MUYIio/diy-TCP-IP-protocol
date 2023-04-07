/**
 * 计数信号量
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "cpu/irq.h"
#include "core/task.h"
#include "ipc/sem.h"
#include "os_cfg.h"

/**
 * 信号量初始化
 */
void sem_init (sem_t * sem, int init_count) {
    sem->count = init_count;
    list_init(&sem->wait_list);
}

/**
 * 申请信号量
 */
void sem_wait (sem_t * sem) {
    irq_state_t  irq_state = irq_enter_protection();

    if (sem->count > 0) {
        sem->count--;
    } else {
        // 从就绪队列中移除，然后加入信号量的等待队列
        task_t * curr = task_current();
        task_set_block(curr);
        list_insert_last(&sem->wait_list, &curr->wait_node);
        task_dispatch();
    }

    irq_leave_protection(irq_state);
}

/**
 * @brief 有延时的申请信号量
 */
int sem_wait_tmo (sem_t * sem, int ms) {
    irq_state_t  irq_state = irq_enter_protection();

    if (sem->count > 0) {
        sem->count--;
    } else {
        // 从就绪队列中移除，同时加入延时队列
        task_t * curr = task_current();
        task_set_block(curr);
        if (ms > 0) {
            task_set_sleep(curr, (ms + (OS_TICK_MS - 1))/ OS_TICK_MS);
        }

        // 同时加入等待队列
        curr->wait_list = &sem->wait_list;
        list_insert_last(&sem->wait_list, &curr->wait_node);

        task_dispatch();
        irq_leave_protection(irq_state);
        return curr->status;
    }

    irq_leave_protection(irq_state);
    return 0;
}

/**
 * 释放信号量
 */
void sem_notify (sem_t * sem) {
    irq_state_t  irq_state = irq_enter_protection();

    if (list_count(&sem->wait_list)) {
        // 有进程等待，则唤醒加入就绪队列
        list_node_t * node = list_remove_first(&sem->wait_list);
        task_t * task = list_node_parent(node, task_t, wait_node);

        // 如果进程同时还延时，先延时队列中移除
        if (task->sleep_ticks > 0) {
            task_set_wakeup(task);
        }

        task_set_ready(task);
        task->status = 0;
        task->wait_list = (list_t *)0;
        task->sleep_ticks = 0;      // 注意设置为0，保护的值可能会在下次唤醒时被误认为非0，造成错误
        task_dispatch();
    } else {
        sem->count++;
    }

    irq_leave_protection(irq_state);
}

/**
 * 获取信号量的当前值
 */
int sem_count (sem_t * sem) {
    irq_state_t  irq_state = irq_enter_protection();
    int count = sem->count;
    irq_leave_protection(irq_state);
    return count;
}

