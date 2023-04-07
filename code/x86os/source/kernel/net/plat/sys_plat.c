/**
 * @file sys_plat.c
 * @author lishutong (527676163@qq.com)
 * @brief 不同操作系统平台的接口
 * @version 0.110
 * @date 2022-12-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "sys_plat.h"

#if defined(SYS_PLAT_X86OS)
#include "dev/time.h"
#include "os_cfg.h"
#include "dbg.h"
#include "sys.h"
#include "mblock.h"
#include "cpu/irq.h"
#include "core/task.h"

#define NET_TASK_NR                 3           // 如果收发均有线程，至少3个。否则1个
#define NET_SEM_NR                  100         // 信号量数量
#define NET_MUTEX_NR                100         // 互斥锁数量

typedef struct _net_task_t {
    task_t task;
    uint32_t stack[10*1024];
}net_task_t;

static net_task_t task_tbl[NET_TASK_NR];
static mblock_t task_mblock;
static sem_t sem_tbl[NET_SEM_NR];
static mblock_t sem_mblock;
static mutex_t mutex_tbl[NET_MUTEX_NR];
static mblock_t mutex_mblock;

void sys_time_curr (net_time_t * time) {
    *time = sys_get_ticks();
}

int sys_time_goes (net_time_t * pre) {
    // 获取当前时间
    net_time_t curr = sys_get_ticks();

    // 记录过去了多少毫秒
   int diff_ms = (curr - *pre) * OS_TICK_MS;

    // 记录下这次调用的时间
    *pre  = curr;
    return diff_ms;    
}

// 计数信号量相关：由具体平台实现
sys_sem_t sys_sem_create(int init_count) {
    sys_sem_t sem = (sys_sem_t)mblock_alloc(&sem_mblock, -1);
    if (sem != SYS_SEM_INVALID) {
       sem_init(sem, init_count);
    }
    return sem;
}

void sys_sem_free(sys_sem_t sem) {
    mblock_free(&sem_mblock, sem);
}

int sys_sem_wait(sys_sem_t sem, uint32_t ms) {
    return sem_wait_tmo(sem, ms);
}

void sys_sem_notify(sys_sem_t sem) {
    sem_notify(sem);
}

// 互斥信号量：由具体平台实现
sys_mutex_t sys_mutex_create(void) {
    sys_mutex_t m = (sys_mutex_t)mblock_alloc(&mutex_mblock, -1);
    if (m != SYS_MUTEx_INVALID) {
        mutex_init(m);
    }
    return m;
}

void sys_mutex_free(sys_mutex_t mutex) {
    mblock_free(&mutex_mblock, mutex);
}

void sys_mutex_lock(sys_mutex_t mutex) {
    mutex_lock(mutex);
}

void sys_mutex_unlock(sys_mutex_t mutex) {
    mutex_unlock(mutex);
}

sys_intlocker_t sys_intlocker_lock(void) {
    return irq_enter_protection();
}

void sys_intlocker_unlock(sys_intlocker_t locker) {
    irq_leave_protection(locker);
}

// 线程相关：由具体平台实现
sys_thread_t sys_thread_create(sys_thread_func_t entry, void* arg) {
    net_task_t * task = (net_task_t *)mblock_alloc(&task_mblock, -1);

    // 初始化系统线程
    int err = task_init(&task->task, "net task", TASK_FLAG_SYSTEM, (uint32_t)entry, (uint32_t)(task->stack + sizeof(task->stack)));
    if (err < 0) {
        log_printf("net: create task failed.");
        return SYS_THREAD_INVALID;
    }

    // 注意启动任务
    task_start(&task->task);
    return &task->task;
}

void sys_thread_exit (int error) {
    // 不实现，加入os内核后，应用层不需要使用该接口
}

sys_thread_t sys_thread_self (void) {
    return task_current();
}

void sys_sleep(int ms) {
    sys_msleep(ms);
}

void sys_plat_init(void) {
    mblock_init(&task_mblock, task_tbl, sizeof(net_task_t), NET_TASK_NR, NLOCKER_NONE);
    mblock_init(&sem_mblock, sem_tbl, sizeof(sem_t), NET_SEM_NR, NLOCKER_NONE);
    mblock_init(&mutex_mblock, mutex_tbl, sizeof(mutex_t), NET_MUTEX_NR, NLOCKER_NONE);
}
	
 // windows
#elif defined(SYS_PLAT_WINDOWS)

#include <winsock.h>
#include <tchar.h>
#include <time.h>
#include "pcap.h"

#pragma comment(lib, "ws2_32.lib")  // 加载win32的网络库

/**
 * 调整npcap的搜索路径：默认安装在系统的dll路径\npcap目录下
 * 设置该路径，以避免使用其它已经安装的winbcap版本的dll
 * 注意：要先安装npcap软件包
 */
int load_pcap_lib() {
    static int dll_loaded = 0;
    _TCHAR  npcap_dir[512];
    int size;

    if (dll_loaded) {
        return 0;
    }

    size = GetSystemDirectory(npcap_dir, 480);
    if (!size) {
        fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
        return -1;
    }

    _tcscat_s(npcap_dir, 512, _T("\\Npcap"));
    if (SetDllDirectory(npcap_dir) == 0) {
        fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
        return -1;
    }

    dll_loaded = 1;
    return 0;
}

/**
 * @brief 获取当前时间
 */
void sys_time_curr (net_time_t * time) {
    // https://learn.microsoft.com/zh-cn/windows/win32/api/sysinfoapi/nf-sysinfoapi-gettickcount?redirectedfrom=MSDN
    *time = GetTickCount();     // 自系统启动以来已用过的毫秒数
}

/**
 * @brief 返回当前时间与传入的time之间时间差值, 调用完成之后，time被更新为当前时间
 * 
 * 第一次调用时，返回的时间差值无效
 */
int sys_time_goes (net_time_t * pre) {
    // 获取当前时间
    net_time_t curr = GetTickCount();

    // 记录过去了多少毫秒
   int diff_ms = curr - *pre;

    // 记录下这次调用的时间
    *pre  = curr;
    return diff_ms;
}

sys_sem_t sys_sem_create(int init_count) {
    return CreateSemaphore(NULL, init_count, 0xFFFF, NULL);
}

void sys_sem_free(sys_sem_t sem) {
    CloseHandle(sem);
}

int sys_sem_wait(sys_sem_t sem, uint32_t tmo_ms) {
    DWORD tmo = (tmo_ms == 0) ? INFINITE : tmo_ms;
    DWORD  err = WaitForSingleObject(sem, tmo);
    if (err == WAIT_OBJECT_0) {
        return 0;
    }

    if (err == WAIT_TIMEOUT) {
        return -4;
    }

    printf("wait error\n");
    return -1;
}

void sys_sem_notify(sys_sem_t sem) {
    ReleaseSemaphore(sem, 1, NULL);
}

/**
 * 创建线程互斥锁
 * @return 创建的互斥信号量
 */
sys_mutex_t sys_mutex_create(void) {
    sys_mutex_t mutex = CreateMutex(NULL, FALSE, NULL); // 初始不被占用
    if (mutex == NULL) {
        return SYS_MUTEx_INVALID;
    }
    return mutex;
}

/**
 * 释放互斥信号量
 * @param mutex
 */
void sys_mutex_free(sys_mutex_t locker) {
    ReleaseMutex(locker);
}

/**
 * 锁定线程互斥锁
 * @param mutex 待锁定的互斥信号量
 */
void sys_mutex_lock(sys_mutex_t locker) {
    WaitForSingleObject(locker, INFINITE);
}

/**
 * 释放线程互斥锁
 * @param mutex 待释放的互斥信号量
 */
void sys_mutex_unlock(sys_mutex_t locker) {
    ReleaseMutex(locker);
}

sys_thread_t sys_thread_create(void (*entry)(void * arg), void* arg) {
    return CreateThread(
        NULL,                           // SD
        0,                              // initial stack size
        (LPTHREAD_START_ROUTINE)entry,  // thread function
        arg,                            // thread argument
        0,                              // 创建后立即运行
        NULL                            // thread identifier
        );
}

/**
 * @brief 结束线程，目前只能结束自己
 */
void sys_thread_exit (int error) {
    ExitThread((DWORD)error);
}

/**
 * @brief 简单的延时，以毫秒为单位
 */
void sys_sleep(int ms) {
    Sleep(ms);
}

void sys_plat_init(void) {
}

#elif defined(SYS_PLAT_LINUX) || defined(SYS_PLAT_MAC)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "pcap.h"

static int load_pcap_lib(void) {
    return 0;
}

/**
 * @brief 获取当前时间
 */
void sys_time_curr (net_time_t * time) {
    gettimeofday(time, NULL);
}

/**
 * @brief 返回当前时间与传入的time之间时间差值, 调用完成之后，time被更新为当前时间
 * 
 * 第一次调用时，返回的时间差值无效
 */
int sys_time_goes (net_time_t * pre) {
    // 获取当前时间
    struct timeval curr;
    gettimeofday(&curr, NULL);

    // 记录过去了多少毫秒
    int diff_ms = (curr.tv_sec - pre->tv_sec) * 1000 + (curr.tv_usec - pre->tv_usec) / 1000;

    // 记录下这次调用的时间
    *pre  = curr;
    return diff_ms;
}

sys_sem_t sys_sem_create(int init_count) {
    sys_sem_t sem = (sys_sem_t)malloc(sizeof(struct _xsys_sem_t));
    if (!sem) {
        return (sys_sem_t)0;
    }

    sem->count = init_count;

    int err = pthread_cond_init(&(sem->cond), NULL);
    if (err) {
        return (sys_sem_t)0;
    }

    err = pthread_mutex_init(&(sem->locker), NULL);
    if (err) {
        return (sys_sem_t)0;
    }

    return sem;
}

/**
 * 释放掉信号量
 */
void sys_sem_free(sys_sem_t sem) {
    pthread_cond_destroy(&(sem->cond));
    pthread_mutex_destroy(&(sem->locker));
    free(sem);
}

/**
 * 等待信号量
 * @param sem 等待的信号量
 * @param tmo 等待的超时时间
 */
int sys_sem_wait(sys_sem_t sem, uint32_t tmo_ms) {
    pthread_mutex_lock(&(sem->locker));

    if (sem->count <= 0) {
        int ret;

        if (tmo_ms > 0) {
            struct timeval curr_time;
            struct timespec ts;

            // 转换为绝对时间等待
            gettimeofday(&curr_time, NULL);
            ts.tv_nsec = (tmo_ms % 1000) * 1000000L;
            ts.tv_sec = tmo_ms / 1000;
            ret = pthread_cond_timedwait_relative_np(&sem->cond, &sem->locker, &ts);
            if (ret == ETIMEDOUT) {
                pthread_mutex_unlock(&(sem->locker));
                return -1;
            }
        } else {
            ret = pthread_cond_wait(&sem->cond, &sem->locker);
            if (ret < 0) {
                pthread_mutex_unlock(&(sem->locker));
                return -1;
            }
        }
    }

    sem->count--;
    pthread_mutex_unlock(&(sem->locker));
    return 0;
}

/**
 * 通知信号量
 * @param sem 待通知的信号量
 */
void sys_sem_notify(sys_sem_t sem) {
    pthread_mutex_lock(&(sem->locker));

    sem->count++;

    // 通知线程，有新的资源可用
    pthread_cond_signal(&(sem->cond));

    pthread_mutex_unlock(&(sem->locker));
}

/**
 * 创建一个线程
 * @param entry 线程的入口函数
 * @param arg 传递给线程的参数
 * @param prio 优先级
 * @param stack_size 堆栈大小
 */
sys_thread_t sys_thread_create(void (*entry)(void * arg), void* arg) {
    // 堆栈和优先级不使用，采用默认设置
    pthread_t * pthread = (pthread_t *)malloc(sizeof(pthread_t));
    if (!pthread) {
        return (pthread_t *)0;
    }

    int err = pthread_create(pthread,
                         NULL,
                         (void* (*)(void * arg))entry,
                         arg);
    if (err) {
        return (pthread_t *)0;
    }

    return pthread;
}

/**
 * 销毁线程
 */
void sys_thread_del_self() {
    pthread_exit(NULL);
}

/**
 * @brief 简单的延时，以毫秒为单位
 */
void sys_sleep(int ms) {
    usleep(1000 * ms);
}

/**
 * 创建线程互斥锁
 * @return 创建的互斥信号量
 */
sys_mutex_t sys_mutex_create(void) {
    sys_mutex_t mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

    int err = pthread_mutex_init(mutex, NULL);
    if (err < 0) {
        return (sys_mutex_t)0;
    }
    return mutex;
}

/**
 * 释放互斥信号量
 * @param mutex
 */
void sys_mutex_free(sys_mutex_t locker) {
    pthread_mutex_destroy(locker);
    free(locker);
}

/**
 * 锁定线程互斥锁
 * @param mutex 待锁定的互斥信号量
 */
void sys_mutex_lock(sys_mutex_t locker) {
    pthread_mutex_lock(locker);
}

/**
 * 释放线程互斥锁
 * @param mutex 待释放的互斥信号量
 */
void sys_mutex_unlock(sys_mutex_t locker) {
    pthread_mutex_unlock(locker);
}

#endif
