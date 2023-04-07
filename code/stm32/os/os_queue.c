#include "os_queue.h"
#include "os_mem.h"
#include "os_list.h"
#include "os_list.h"
#include <string.h>

/**
 * 消息等待的类型
 */
typedef struct _queue_wait_t {
    void * msg;
    int opt;
}queue_wait_t;

/**
 * 初始化消息队列
 * @param queue 待初始化的消息队列
 * @param size 队列大小
 * @param msg_buf 消息存储的位置
 * @param flags 相关标志
*/
static os_err_t queue_init (os_queue_t * queue, void * buf, int msg_size, int msg_cnt, int flags) {
    // 初始化等待结构
    os_err_t err = os_event_init(&queue->in_event, OS_EVENT_TYPE_QUEUE, flags);
    if (err < 0) {
        os_dbg("create queue in_event failed: %s.", os_err_str(err));
        return err;
    }
    // 初始化等待结构
    err = os_event_init(&queue->out_event, OS_EVENT_TYPE_QUEUE, flags);
    if (err < 0) {
        os_event_uninit(&queue->in_event);
        os_dbg("create queue out_event failed: %s.", os_err_str(err));
        return err;
    }

    queue->read = queue->write = 0;
    queue->msg_cnt = 0;
    queue->msg_size = msg_size;
    queue->msg_max = msg_cnt;
    queue->buf = buf;
    return OS_ERR_OK;
}

/**
 * 初始化消息队列
 * @param queue 待初始化的消息队列
 * @param size 队列大小
 * @param msg_buf 消息存储的位置
 * @return 初始化结果
*/
os_err_t os_queue_init (os_queue_t * queue, void * buf, int msg_size, int msg_cnt) {
    os_param_failed(queue == OS_NULL, OS_ERR_PARAM);
    os_param_failed(buf == OS_NULL, OS_ERR_PARAM);
    os_param_failed(msg_size <= 0, OS_ERR_PARAM);
    os_param_failed(msg_cnt <= 0, OS_ERR_PARAM);
    return queue_init(queue, buf, msg_size, msg_cnt, 0);
}

/**
 * 取消消息队列的初始化
 * @param queue 消息队列
*/
os_err_t os_queue_uninit (os_queue_t * queue) {       
    os_param_failed(queue == OS_NULL, OS_ERR_PARAM);

    // 唤醒所有任务后，尝试进行调试
    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&queue->in_event) + os_event_wait_cnt(&queue->out_event);
    os_event_uninit(&queue->in_event);
    os_event_uninit(&queue->out_event);
    os_leave_protect(protect);

    // 有任务唤醒时，进行调度，以便运行更高优先级的任务
    if (cnt > 0) {
        os_sched_run();
    } 

    return OS_ERR_OK;
}

/**
 * 分配一个消息队列
 * @param size 消息队列的大小
 * @return 创建的消息队列
 */
os_queue_t * os_queue_create (int msg_size, int msg_cnt) {
    os_param_failed(msg_size <= 0, OS_NULL);
    os_param_failed(msg_cnt <= 0, OS_NULL);

    // 分配内存
    os_queue_t * queue = os_mem_alloc(sizeof(os_queue_t));
    if (queue == OS_NULL) {
        os_dbg("error: alloc queue failed");
        return OS_NULL;
    }

    // 分配存储空间，注意size对齐到int大小的边界
    void * buf = os_mem_alloc(msg_size * msg_cnt);
    if (buf == OS_NULL) {
        os_dbg("error: alloc msg_buf failed");
        os_mem_free(queue);
        return OS_NULL;
    }

    // 初始化消息队列结构
    os_err_t err = queue_init(queue, buf, msg_size, msg_cnt, OS_EVENT_FLAG_ALLOC);
    if (err < 0) {
        os_dbg("error: init queue failed.");
        os_mem_free(queue);
        return OS_NULL;
    }

    return queue;
}

/**
 * 释放一个消息队列
 * @param 需要释放的消息队列
 */
os_err_t os_queue_free (os_queue_t * queue) {
    os_param_failed(queue == OS_NULL, OS_ERR_PARAM);

    os_queue_uninit(queue);
    if (queue->in_event.flags & OS_EVENT_FLAG_ALLOC) {
        os_mem_free(queue->buf);
        os_mem_free(queue);
    }

    return OS_ERR_OK;
}

/**
 * 获取消息数量
 * @param queue 消息队列
 * @return 消息数量
*/
int os_queue_msg_cnt (os_queue_t * queue) {
    os_param_failed(queue == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = queue->msg_cnt;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取消息队列的大小
 * @param queue 存储块结构
 * @return 队列大小
*/
int os_queue_free_cnt (os_queue_t * queue) {
    os_param_failed(queue == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = queue->msg_max - queue->msg_cnt;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取等待消息的任务数量
 * @param qeueue 消息队列
 * @return 等待消息的任务数量
 */
int os_queue_wait_tasks(os_queue_t * queue) {
    os_param_failed(queue == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&queue->out_event);
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取等待释放消息的任务数量
 * @param qeueue 消息队列
 * @return 等待消息的任务数量
 */
int os_queue_release_tasks(os_queue_t * queue) {
    os_param_failed(queue == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&queue->in_event);
    os_leave_protect(protect);
    return cnt;
}

/**
 * 清空消息队列
 * @param queue 待清空的消息队列
 */
os_err_t os_queue_clear (os_queue_t * queue) {
    os_param_failed(queue == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    os_event_clear(&queue->in_event, OS_NULL, OS_ERR_REMOVE);
    os_event_clear(&queue->out_event, OS_NULL, OS_ERR_REMOVE);
    os_leave_protect(protect);
    return OS_ERR_OK;
}

/**
 * 从消息队列中读取一个消息到指定的位置
 */
static void queue_copy (os_queue_t * queue, void * dest, void * src) {
    switch (queue->msg_size) {
    case 1:
        *(uint8_t *)dest = *(uint8_t *)src;
        break;
    case 2:
        *(uint16_t *)dest = *(uint16_t *)src;
        break;
    case 4:
        *(uint32_t *)dest = *(uint32_t *)src;
        break;
    default:
        memcpy(dest, src, queue->msg_size);
        break;
    }
}

/**
 * 从消息队列中读取一个消息到指定的位置
 */
static void queue_read_out (os_queue_t * queue, int opt, void * msg) {
    void * buf = (uint8_t *)queue->buf + queue->read;
    switch (queue->msg_size) {
    case 1:
        *(uint8_t *)msg = *(uint8_t *)buf;
        break;
    case 2:
        *(uint16_t *)msg = *(uint16_t *)buf;
        break;
    case 4:
        *(uint32_t *)msg = *(uint32_t *)buf;
        break;
    default:
        memcpy(msg, buf, queue->msg_size);
        break;
    }

    queue->read += queue->msg_size;
    if (queue->read >= queue->msg_max * queue->msg_size) {
        queue->read = 0;
    }
    queue->msg_cnt--;
}

/**
 * 向消息队列中写入一个消息
 * 消息可以往队列头部也可以往队列尾部写消息
 */
static void queue_write_in (os_queue_t * queue, void * msg, int opt) {
    if (opt & OS_QUEUE_RELEASE_FRONT) {
        queue->read -= queue->msg_size;
        if (queue->read < 0) {
            queue->read = (queue->msg_max - 1) * queue->msg_size;
        }

        void * buf = (uint8_t *)queue->buf + queue->read;
        switch (queue->msg_size) {
        case 1:
            *(uint8_t *)buf = *(uint8_t *)msg;
            break;
        case 2:
            *(uint16_t *)buf = *(uint16_t *)msg;
            break;
        case 4:
            *(uint32_t *)buf = *(uint32_t *)msg;
            break;
        default:
            memcpy(buf, msg, queue->msg_size);
            break;
        }
    } else {
        void * buf = (uint8_t *)queue->buf + queue->write;
        switch (queue->msg_size) {
        case 1:
            *(uint8_t *)buf = *(uint8_t *)msg;
            break;
        case 2:
            *(uint16_t *)buf = *(uint16_t *)msg;
            break;
        case 4:
            *(uint32_t *)buf = *(uint32_t *)msg;
            break;
        default:
            memcpy(buf, msg, queue->msg_size);
            break;
        }

        queue->write += queue->msg_size;
        if (queue->write >= queue->msg_max * queue->msg_size) {
            queue->write = 0;
        }
    }
    queue->msg_cnt++;
}

/**
 * 等待消息队列中的消息
 * @param queue 等待的消息队列
 * @param ms 等待的毫秒数
 * @param msg 取得的消息
*/
os_err_t os_queue_wait (os_queue_t * queue, int ms, int opt, void * msg) {
    os_param_failed(queue == OS_NULL, OS_ERR_PARAM);
    os_param_failed(ms < 0, OS_ERR_PARAM);
    os_param_failed(msg == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    if (queue->msg_cnt > 0) {
        // 如果有消息，取出一个消息
        queue_read_out(queue, opt, msg);

        // 唤醒等待写消息的线程，将其需要写入的消息写到队列中，消息总数不变
        os_task_t * task = os_event_wakeup(&queue->in_event);
        if (task) {
            os_wait_t * wait = task->wait;
            queue_wait_t * q_wait = (queue_wait_t *)wait->reason;
            wait->err = OS_ERR_OK;
            task->wait = OS_NULL;
            queue_write_in(queue, q_wait->msg, q_wait->opt);
        }

        os_leave_protect(protect);
        return OS_ERR_OK;
    } else if (ms < 0) {
        os_dbg("msg count == 0 and no wait\n");
        os_leave_protect(protect);
        return OS_ERR_NONE;
    } else {
        os_leave_protect(protect);

        queue_wait_t q_wait = {
            .opt = 0, 
            .msg = msg
        };

        // 没有，则等待可用的资源
        os_err_t err = os_event_wait(&queue->out_event, &q_wait, ms);
        return err;
    }
}

/**
 * 释放消息
 * @param queue 消息队列
 * @param msg 待释放的消息
 * @return 释放的结果
 */
os_err_t os_queue_release (os_queue_t * queue, int ms, int opt, void * msg) {
    os_param_failed(queue == OS_NULL, OS_ERR_PARAM);
    os_param_failed(msg == OS_NULL, OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    os_task_t * task = os_event_wakeup(&queue->out_event);
    if (task) {
        // 有任务等消息，将消息发给等待的任务
        os_wait_t * wait = task->wait;
        queue_wait_t * q_wait = (queue_wait_t *)wait->reason;
        wait->err = OS_ERR_OK;
        task->wait = OS_NULL;

        // 直接消息复制，不用经过缓存
        queue_copy(queue, q_wait->msg, msg);
        os_leave_protect(protect);
        return OS_ERR_OK;
    } else if (queue->msg_max > queue->msg_cnt) {
        // 有空闲空间，写入队列中
        queue_write_in(queue,msg, opt);

        os_leave_protect(protect);
        return OS_ERR_OK;
    } if (ms < 0) {
        // 无空闲，不等待
        os_leave_protect(protect);

        os_dbg("queue full and no wait\n");
        return OS_ERR_FULL;
    } else {
        os_leave_protect(protect);

        // 无空闲，等待空闲空间写入
        queue_wait_t q_wait = {
            .opt = opt, 
            .msg = msg
        };

        os_err_t err = os_event_wait(&queue->in_event, &q_wait, ms);
        return err;
    }
}

