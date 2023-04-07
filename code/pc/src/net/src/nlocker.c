/**
 * @file nlocker.c
 * @author lishutong (527676163@qq.com)
 * @brief 资源锁
 * @version 0.1
 * @date 2022-10-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "nlocker.h"

/**
 * 初始化资源锁
 */
net_err_t nlocker_init(nlocker_t * locker, nlocker_type_t type) {
    if (type == NLOCKER_THREAD) {
        sys_mutex_t mutex = sys_mutex_create();
        if (mutex == SYS_MUTEx_INVALID) {
            return NET_ERR_SYS;
        }
        locker->mutex = mutex;
    }

    locker->type = type;
    return NET_ERR_OK;
}

/**
 * @brief 销毁锁
 */
void nlocker_destroy(nlocker_t * locker) {
    if (locker->type == NLOCKER_THREAD) {
        sys_mutex_free(locker->mutex);
    }
}

/**
 * @brief 上锁
 */
void nlocker_lock(nlocker_t * locker) {
    if (locker->type == NLOCKER_THREAD) {
        sys_mutex_lock(locker->mutex);
    }
}

/**
 * @brief 解锁
 */
void nlocker_unlock(nlocker_t * locker) {
    if (locker->type == NLOCKER_THREAD) {
        sys_mutex_unlock(locker->mutex);
    } else if (locker->type == NLOCKER_INT) {
#if NETIF_USE_INT
        sys_intlocker_unlock(locker->state);
#endif
    }
}

