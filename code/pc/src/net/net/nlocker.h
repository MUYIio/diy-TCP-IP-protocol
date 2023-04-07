/**
 * @file nlocker.h
 * @author lishutong (527676163@qq.com)
 * @brief 资源锁
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef NLOCKER_H
#define NLOCKER_H

#include "net_plat.h"
#include "sys.h"

/**
 * @brief 锁的类型
 *
 */
typedef enum _nlocker_type_t {
    NLOCKER_NONE,                   // 不需要锁
    NLOCKER_THREAD,                 // 用于线程共享的锁
    NLOCKER_INT,                    // 中断相关的锁
}nlocker_type_t;

typedef struct _nlocker_t {
    nlocker_type_t type;                // 锁的类型

    // 根据不同的锁类型，放置不同的结构
    union {
        sys_mutex_t mutex;           // 用于线程之间访问的互斥锁
#if NETIF_USE_INT == 1
        sys_intlocker_t state;      // 中断锁
#endif
    };
}nlocker_t;

net_err_t nlocker_init(nlocker_t * locker, nlocker_type_t type);
void nlocker_destroy(nlocker_t * locker);
void nlocker_lock(nlocker_t * locker);
void nlocker_unlock(nlocker_t * locker);

#endif // NLOCKER_H
