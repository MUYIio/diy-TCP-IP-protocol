/**
 * @file fixq.c
 * @brief 定长消息队列
 * @brief
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 * 在整个协议栈中，数据包需要排队，线程之间的通信消息也需要排队，因此需要
 * 借助于消息队列实现。该消息队列长度是定长的，以避免消息数量太多耗费太多资源
 */
#include "fixq.h"
#include "nlocker.h"
#include "dbg.h"
#include "sys.h"

/**
 * @brief 初始化定长消息队列
 */
net_err_t fixq_init(fixq_t *q, void **buf, int size, nlocker_type_t type) {
    q->size = size;
    q->in = q->out = q->cnt = 0;
    q->buf = (void **)0;
    q->recv_sem = SYS_SEM_INVALID;
    q->send_sem = SYS_SEM_INVALID;

    // 创建锁
    net_err_t err = nlocker_init(&q->locker, type);
    if (err < 0) {
        dbg_error(DBG_QUEUE, "init locker failed!");
        return err;
    }

    // 创建发送信号量
    q->send_sem = sys_sem_create(size);
    if (q->send_sem == SYS_SEM_INVALID)  {
        dbg_error(DBG_QUEUE, "create sem failed!");
        err = NET_ERR_SYS;
        goto init_failed;
    }

    q->recv_sem = sys_sem_create(0);
    if (q->recv_sem == SYS_SEM_INVALID) {
        dbg_error(DBG_QUEUE, "create sem failed!");
        err = NET_ERR_SYS;
        goto init_failed;
    }

    q->buf = buf;
    return NET_ERR_OK;
init_failed:
    if (q->recv_sem != SYS_SEM_INVALID) {
        sys_sem_free(q->recv_sem);
    }

    if (q->send_sem != SYS_SEM_INVALID) {
        sys_sem_free(q->send_sem);
    }

    nlocker_destroy(&q->locker);
    return err;
}

/**
 * @brief 向消息队列写入一个消息
 * 如果消息队列满，则看下tmo，如果tmo < 0则不等待
 */
net_err_t fixq_send(fixq_t *q, void *msg, int tmo) {
    nlocker_lock(&q->locker);
    if ((q->cnt >= q->size) && (tmo < 0)) {
        // 如果缓存已满，并且不需要等待，则立即退出
        nlocker_unlock(&q->locker);
        return NET_ERR_FULL;
    }
    nlocker_unlock(&q->locker);

    // 消耗掉一个空闲资源，如果为空则会等待
    if (sys_sem_wait(q->send_sem, tmo) < 0) {
        return NET_ERR_TMO;
    }

    // 有空闲单元写入缓存
    nlocker_lock(&q->locker);
    q->buf[q->in++] = msg;
    if (q->in >= q->size) {
        q->in = 0;
    }
    q->cnt++;
    nlocker_unlock(&q->locker);

    // 通知其它进程有消息可用
    sys_sem_notify(q->recv_sem);
    return NET_ERR_OK;
}
