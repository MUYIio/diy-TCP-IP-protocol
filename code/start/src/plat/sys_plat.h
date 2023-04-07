/**
 * @file sys_plat.h
 * @author lishutong (527676163@qq.com)
 * @brief 不同操作系统平台的接口
 * @version 0.110
 * @date 2022-12-19
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef SYS_PLAT_H
#define SYS_PLAT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// 系统硬件配置
// 不同网卡配置，共2块网卡
#if 1
static const char netdev0_ip[] = "192.168.74.2";
static const char netdev0_gw[] = "192.168.74.1";
static const char friend0_ip[] = "192.168.74.3";
static const char netdev0_phy_ip[] = "192.168.74.1";    // 用于收发包的真实网卡ip地址，在qemu上不需要使用
static const char netdev0_mask[] = "255.255.255.0";
static const uint8_t netdev0_hwaddr[] = { 0x00, 0x50, 0x56, 0xc0, 0x00, 0x11 };
#else
static const char netdev0_ip[] = "192.168.74.2";
static const char netdev0_gw[] = "192.168.74.3";
static const char friend0_ip[] = "192.168.74.3";
static const char netdev0_phy_ip[] = "192.168.74.1";    // 用于收发包的真实网卡ip地址
static const char netdev0_mask[] = "255.255.255.0";
#endif

static const char netdev1_ip[] = "10.0.2.200";
static const char netdev1_gw[] = "10.0.2.2";
static const char netdev1_phy_ip[] = "192.168.254.1";
static const char friend1_ip[] = "10.0.2.2";
static const char netdev1_mask[] = "255.255.255.0";
static const uint8_t netdev1_hwaddr[] = { 0x00, 0x50, 0x56, 0xc0, 0x00, 0x22 };

#ifndef __GNUC__
typedef long ssize_t;
#endif

#if defined(SYS_PLAT_X86OS)

#include "ipc/sem.h"
#include "ipc/mutex.h"
#include "core/task.h"
#include "tools/klib.h"
#include "tools/log.h"

typedef uint32_t net_time_t;      // 时间类型

#define SYS_THREAD_INVALID          (task_t *)0
#define SYS_SEM_INVALID             (sem_t *)0
#define SYS_MUTEx_INVALID           (mutex_t *)0

typedef mutex_t * sys_mutex_t;        // 互斥锁
typedef task_t * sys_thread_t;        // 线程
typedef sem_t * sys_sem_t;            // 信号量

#define plat_strlen         kernel_strlen
#define plat_strcpy         kernel_strcpy
#define plat_strncpy        kernel_strncpy
#define plat_strcmp         kernel_strcmp
#define plat_stricmp        kernel_stricmp
#define plat_memset         kernel_memset
#define plat_memcpy         kernel_memcpy
#define plat_memcmp         kernel_memcmp
#define plat_sprintf        kernel_sprintf
#define plat_vsprintf       kernel_vsprintf
#define plat_printf         log_printf

#elif defined(SYS_PLAT_WINDOWS)
// pcap引用了winsock.h，下面用了windows，有些宏重复了
// 加上下面的宏可避免编译出错
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <pcap.h>
#include <stdio.h>
#include <string.h>

typedef DWORD net_time_t;      // 时间类型

#define SYS_THREAD_INVALID          (HANDLE)0
#define SYS_SEM_INVALID             (HANDLE)0
#define SYS_MUTEx_INVALID           (HANDLE)0

typedef HANDLE sys_mutex_t;         // 互斥锁
typedef HANDLE sys_thread_t;        // 线程
typedef HANDLE sys_sem_t;           // 信号量

#define plat_strlen         strlen
#define plat_strcpy         strcpy
#define plat_strncpy        strncpy
#define plat_strcmp         strcmp
#define plat_stricmp        _stricmp
#define plat_memset         memset
#define plat_memcpy         memcpy
#define plat_memcmp         memcmp
#define plat_sprintf        sprintf
#define plat_vsprintf       vsprintf
#define plat_printf         printf

// PCAP网卡驱动相关函数
int pcap_find_device(const char* ip, char* name_buf);
int pcap_show_list(void);
pcap_t * pcap_device_open(const char* ip, const uint8_t* mac_addr);

#elif defined(SYS_PLAT_LINUX) || defined(SYS_PLAT_MAC)

#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <pcap.h>
#include <string.h>
#include <stdlib.h>

typedef struct timeval net_time_t;      // 时间类型

#define SYS_THREAD_INVALID          (sys_thread_t)0
#define SYS_SEM_INVALID             (sys_sem_t)0
#define SYS_MUTEx_INVALID           (sys_mutex_t)0

#define plat_strlen         strlen
#define plat_strcpy         strcpy
#define plat_strncpy        strncpy
#define plat_strcmp         strcmp
#define plat_stricmp        strcasecmp
#define plat_memset         memset
#define plat_memcpy         memcpy
#define plat_memcmp         memcmp
#define plat_sprintf        sprintf
#define plat_vsprintf       vsprintf
#define plat_printf         printf

typedef struct _xsys_sem_t {
    int count;                          // 信号量计数
    pthread_cond_t cond;                // 条件变量
    pthread_mutex_t locker;             // 访问C的互斥锁
} * sys_sem_t;

typedef pthread_t sys_thread_t;           // 线程重定义
typedef pthread_mutex_t * sys_mutex_t;      // 互斥信号量

// PCAP网卡驱动相关函数
int pcap_find_device(const char* ip, char* name_buf);
int pcap_show_list(void);
pcap_t * pcap_device_open(const char* ip, const uint8_t* mac_addr);

#else
    #error "Unkonw platform"
#endif // Unix/Linux

sys_sem_t sys_sem_create(int init_count);
void sys_sem_free(sys_sem_t sem);
int sys_sem_wait(sys_sem_t sem, uint32_t ms);
void sys_sem_notify(sys_sem_t sem);

// 互斥信号量：由具体平台实现
sys_mutex_t sys_mutex_create(void);
void sys_mutex_free(sys_mutex_t mutex);
void sys_mutex_lock(sys_mutex_t mutex);
void sys_mutex_unlock(sys_mutex_t mutex);
int sys_mutex_is_valid(sys_mutex_t mutex);

// 线程相关：由具体平台实现
typedef void (*sys_thread_func_t)(void * arg);
sys_thread_t sys_thread_create(sys_thread_func_t entry, void* arg);
void sys_thread_exit (int error);
void sys_sleep(int ms);
sys_thread_t sys_thread_self (void);

void sys_plat_init(void);


#endif // SYS_PLAT_H
