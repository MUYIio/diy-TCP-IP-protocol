#ifndef OS_QUEUE_H
#define OS_QUEUE_H

#include "os_cfg.h"
#include "os_event.h"

/**
 * 消息队列结构
 */
typedef struct _os_queue_t {
    os_event_t in_event;            // 写事件结构
    os_event_t out_event;           // 读事件结构
    int msg_cnt, msg_size;          // 当前数量，大小
    int msg_max;                    // 最大消息数量
    int read, write;                // 读写索引
    void * buf;                     // 消息缓存
}os_queue_t;

#define OS_QUEUE_RELEASE_FRONT      (1 << 0)    // 消息发送至头部
#define OS_QUEUE_RELEASE_BROADCAST  (1 << 1)    // 发送广播消息

os_err_t os_queue_init (os_queue_t * queue, void * msg_buf, int msg_size, int msg_cnt);
os_err_t os_queue_uninit(os_queue_t * queue);
os_queue_t * os_queue_create (int msg_size, int msg_cnt);
os_err_t os_queue_free (os_queue_t * queue);
os_err_t os_queue_wait (os_queue_t * queue, int ms, int opt, void * msg);
os_err_t os_queue_release (os_queue_t * queue, int ms, int opt, void * msg);
int os_queue_msg_cnt (os_queue_t * queue);
int os_queue_free_cnt (os_queue_t * queue);
int os_queue_wait_tasks(os_queue_t * queue);
int os_queue_release_tasks (os_queue_t * queue);
os_err_t os_queue_clear (os_queue_t * queue);

#endif /* TMBOX_H */ 
