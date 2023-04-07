/**
 * @file fixq.h
 * @brief 定长消息队列
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 * 在整个协议栈中，数据包需要排队，线程之间的通信消息也需要排队，因此需要
 * 借助于消息队列实现。该消息队列长度是定长的，以避免消息数量太多耗费太多资源
 */
#ifndef FIXQ_H
#define FIXQ_H

#include "nlist.h"
#include "nlocker.h"
#include "net_plat.h"

/**
 * 固定长度的消息队列
 */
typedef struct _fixq_t{
    int size;                               // 消息容量
    void** buf;                             // 消息缓存
    int in, out, cnt;                       // 读写位置索引，消息数
    nlocker_t locker;                       // 访问的锁
    sys_sem_t recv_sem;                     // 接收消息时使用的信号量
    sys_sem_t send_sem;                     // 发送消息时使有的信号量
}fixq_t;

net_err_t fixq_init(fixq_t * q, void ** buf, int size, nlocker_type_t share_type);
net_err_t fixq_send(fixq_t* q, void* msg, int tmo);
void * fixq_recv(fixq_t* q, int tmo);
void fixq_destroy(fixq_t * q);
int fixq_count (fixq_t *q);

#endif // fixq_H
