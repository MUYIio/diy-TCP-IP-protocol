/**
 * 本源码配套的课程为 - 自己动手写TCIP/IP协议栈 源码
 * 作者：李述铜
 * 课程网址：http://01ketang.cc
 * 版权声明：本源码非开源，二次开发，或其它商用前请联系作者。
 * 注：源码不断升级中，该版本可能非最新版。如需获取最新版，请联系作者。
 */
#ifndef UDP_H
#define UDP_H

#include "sock.h"

/**
 * UDP控制块
 */
typedef struct _udp_t {
    sock_t  base;                   // 基础控制块
    
    // 没有用定长消息队列，目的是减少开销。定长消息队列要求有缓存表
    // 再者还会创建多个信号量等结构，资源开锁大一些
    nlist_t recv_list;           	// 接收队列
    sock_wait_t rcv_wait;           // 接收等待结构
}udp_t;

net_err_t udp_init(void);

#endif // UDP_H
