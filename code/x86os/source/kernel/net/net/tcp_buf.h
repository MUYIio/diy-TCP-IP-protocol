/**
 * @file tcp_buf.h
 * @author lishutong 527676163@qq.com
 * @brief TCP收发缓存，使用环形缓存实现
 * @version 0.1
 * @date 2022-11-11
 * 
 * @copyright Copyright (c) 2022
 */
#include "net_cfg.h"
#include "sys.h"
#include "pktbuf.h"

/**
 * @brief TCP环形缓存
 */
typedef struct _tcp_sbuf_t {
    int count;                          // 缓存中已有的数据量
    int in, out;                        // 写入读取索引
    int size;                           // 整个缓存空间的
    uint8_t * data;                     // 缓存数据区
}tcp_buf_t;

void tcp_buf_init(tcp_buf_t* buf, uint8_t * data, int size);
void tcp_buf_read_send(tcp_buf_t * src, int offset, pktbuf_t * dest, int count);
void tcp_buf_write_send(tcp_buf_t * dest, const uint8_t * buffer, int len);
int tcp_buf_write_rcv(tcp_buf_t * dest, int offset, pktbuf_t * src, int size);
int tcp_buf_read_rcv (tcp_buf_t * src, uint8_t * buf, int size);
int tcp_buf_remove(tcp_buf_t * buf, int cnt);

/**
 * @brief 获取缓存大小 
 */
static inline int tcp_buf_size (tcp_buf_t * buf) {
    return buf->size;
}

/**
 * @brief 计算空间单元的数量
 */
static inline int tcp_buf_free_cnt(tcp_buf_t * buf) {
    return buf->size - buf->count;
}

/**
 * @brief 返回缓存里的数据量
 */
static inline int tcp_buf_cnt (tcp_buf_t * buf) {
    return buf->count;
}