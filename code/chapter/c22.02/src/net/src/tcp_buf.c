
/**
 * @file tcp_buf.c
 * @author lishutong 527676163@qq.com
 * @brief TCP环形缓冲区
 * @version 0.1
 * @date 2022-11-11
 * 
 * @copyright Copyright (c) 2022
 */
#include "tcp_buf.h"
#include "pktbuf.h"
#include "dbg.h"

/**
 * @brief 初始化tcp缓存
 * 缓存空间自己不分配，而是使用外部传输的空间
 */
void tcp_buf_init(tcp_buf_t* buf, uint8_t * data, int size) {
    buf->in = buf->out = 0;
    buf->count = 0;
    buf->size = size;
    buf->data = data;
}
