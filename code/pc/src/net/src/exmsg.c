/**
 * @brief TCP/IP核心线程通信模块
 * 此处运行了一个核心线程，所有TCP/IP中相关的事件都交由该线程处理
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#include "net_plat.h"
#include "exmsg.h"
#include "fixq.h"
#include "mblock.h"
#include "dbg.h"
#include "nlocker.h"
#include "timer.h"
#include "ipv4.h"
#include "sys.h"

static void * msg_tbl[EXMSG_MSG_CNT];  // 消息缓冲区
static fixq_t msg_queue;            // 消息队列
static exmsg_t msg_buffer[EXMSG_MSG_CNT];  // 消息块
static mblock_t msg_block;          // 消息分配器

/**
 * @brief 收到来自网卡的消息
 */
net_err_t exmsg_netif_in(netif_t* netif) {
    // 分配一个消息结构
    exmsg_t* msg = mblock_alloc(&msg_block, -1);
    if (!msg) {
        dbg_warning(DBG_MSG, "no free exmsg");
        return NET_ERR_MEM;
    }

    // 写消息内容
    msg->type = NET_EXMSG_NETIF_IN;
    msg->netif.netif = netif;

    // 发送消息
    net_err_t err = fixq_send(&msg_queue, msg, -1);
    if (err < 0) {
        dbg_warning(DBG_MSG, "fixq full");
        mblock_free(&msg_block, msg);
        return err;
    }

    return NET_ERR_OK;
}

/**
 * @brief 网络接口有数据到达时的消息处理
 */
static net_err_t do_netif_in(exmsg_t* msg) {
    netif_t* netif = msg->netif.netif;

    // 反复从接口中取出包，然后一次性处理
    pktbuf_t* buf;
    while ((buf = netif_get_in(netif, -1))) {
        dbg_info(DBG_MSG, "recv a packet");

        // 如果有链路层驱动，先将链路层进行处理
        net_err_t err;
        if (netif->link_layer) {
            err = netif->link_layer->in(netif, buf);

            // 发生错误，需要自己释放包。因为这个包是已经在队列中，由自己取出的
            // 因此，如果上层处理不了，需要自己进行释放
            if (err < 0) {
                // 暂不处理，只是回收
                pktbuf_free(buf);
                dbg_warning(DBG_MSG, "netif in failed. err=%d", err);
            }
        } else {
            err = ipv4_in(netif, buf);
            if (err < 0) {
                pktbuf_free(buf);
                dbg_warning(DBG_MSG, "netif in failed. err=%d", err);
            };
        }
    }

    return NET_ERR_OK;
}

/**
 * @brief 执行内部工作函数调用
 */
net_err_t exmsg_func_exec(exmsg_func_t func, void * param) {
    // 构造消息
    func_msg_t func_msg;
    func_msg.thread = sys_thread_self();
    func_msg.func = func;
    func_msg.param = param;
    func_msg.err = NET_ERR_OK;
    func_msg.wait_sem = sys_sem_create(0);
    if (func_msg.wait_sem == SYS_SEM_INVALID) {
        dbg_error(DBG_MSG, "error create wait sem");
        return NET_ERR_MEM;
    }

    // 分配消息结构
    exmsg_t* msg = (exmsg_t*)mblock_alloc(&msg_block, 0);
    msg->type = NET_EXMSG_FUN; 
    msg->func = &func_msg;

    // 发消息给工作线程去执行
    dbg_info(DBG_MSG, "1.begin call func: %p", func);
    net_err_t err = fixq_send(&msg_queue, msg, 0);
    if (err < 0) {
        dbg_error(DBG_MSG, "send msg to queue ailed. err = %d", err);
        mblock_free(&msg_block, msg);
        sys_sem_free(func_msg.wait_sem);
        return err;
    }

    // 等待执行完成
    sys_sem_wait(func_msg.wait_sem, 0);
    dbg_info(DBG_MSG, "4.end call func: %p", func);

    // 释放信号量
    sys_sem_free(func_msg.wait_sem);
    return func_msg.err;
}

/**
 * @brief 执行工作函数
 */
static net_err_t do_func(func_msg_t* func_msg) {
    dbg_info(DBG_MSG, "2.calling func");
    
    func_msg->err = func_msg->func(func_msg);

    sys_sem_notify(func_msg->wait_sem);
    dbg_info(DBG_MSG, "3.func exec complete");
    return NET_ERR_OK;
}

/**
 * @brief 核心线程通信初始化
 */
net_err_t exmsg_init(void) {
    dbg_info(DBG_MSG, "exmsg init");

    // 初始化消息队列
    net_err_t err = fixq_init(&msg_queue, msg_tbl, EXMSG_MSG_CNT, NETIF_USE_INT ? NLOCKER_INT : NLOCKER_THREAD);
    if (err < 0) {
        dbg_error(DBG_MSG, "fixq init error");
        return err;
    }

    // 初始化消息分配器
    err = mblock_init(&msg_block, msg_buffer, sizeof(exmsg_t), EXMSG_MSG_CNT, NETIF_USE_INT ? NLOCKER_INT : NLOCKER_THREAD);
    if (err < 0) {
        dbg_error(DBG_MSG,  "mblock init error");
        return err;
    }

    // 初始化完成
    dbg_info(DBG_MSG, "init done.");
    return NET_ERR_OK;
}

/**
 * @brief 核心线程功能体
 */
static void work_thread (void * arg) {
    // 注意要加上\n。否则由于C库缓存的关系，字符串会被暂时缓存而不输出显示
    dbg_info(DBG_MSG, "exmsg is running....\n");

    // 先调用一下，以便获取初始时间
    net_time_t time;
    sys_time_curr(&time);

    int time_last = TIMER_SCAN_PERIOD;
    while (1) {
        // 有时间等待的等消息，这样就能够及时检查定时器也能同时检查定时消息
        int first_tmo = net_timer_first_tmo();
        exmsg_t* msg = (exmsg_t*)fixq_recv(&msg_queue, first_tmo);
        
        // 计算相比之前过去了多少时间
        int diff_ms = sys_time_goes(&time);
        time_last -= diff_ms;
        if (time_last < 0) {
            // 不准确，但是够用了，不需要那么精确
            net_timer_check_tmo(diff_ms);
            time_last = TIMER_SCAN_PERIOD;
       }
       
        if (msg) {
            // 消息到了，打印提示
            dbg_info(DBG_MSG, "recieve a msg(%p): %d", msg, msg->type);
            switch (msg->type) {
            case NET_EXMSG_NETIF_IN:          // 网络接口消息
                do_netif_in(msg);
                break;
            case NET_EXMSG_FUN:               // API消息
                do_func(msg->func);
                break;
            }

            // 释放消息
            mblock_free(&msg_block, msg);
        }
    }
}

/**
 * @brief 启动核心线程通信机制
 */
net_err_t exmsg_start(void) {
    // 创建核心线程
    sys_thread_t thread = sys_thread_create(work_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }

    return NET_ERR_OK;
}