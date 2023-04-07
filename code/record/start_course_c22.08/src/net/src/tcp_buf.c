#include "tcp_buf.h"
#include "dbg.h"

void tcp_buf_init (tcp_buf_t * buf, uint8_t * data, int size) {
    buf->in = buf->out = 0;
    buf->count = 0;
    buf->size = size;
    buf->data = data;
}

void tcp_buf_write_send (tcp_buf_t * buf, const uint8_t * buffer, int len) {
    while (len > 0) {
        buf->data[buf->in++] = *buffer++;
        if (buf->in >= buf->size) {
            buf->in = 0;
        }
        len--;
        buf->count++;
    }
}

void tcp_buf_read_send (tcp_buf_t * buf, int offset, pktbuf_t * dest, int count) {
    // 超过要求的数据量，进行调整
    int free_for_us = buf->count - offset;      // 跳过offset之前的数据
    if (count > free_for_us) {
        dbg_warning(DBG_TCP, "resize for send: %d -> %d", count, free_for_us);
        count = free_for_us;
    }

    // 复制过程中要考虑buf中的数据回绕的问题
    int start = buf->out + offset;     // 注意拷贝的偏移
    if (start >= buf->size) {
        start -= buf->size;
    }

    while (count > 0) {
        // 当前超过末端，则只拷贝到末端的区域
        int end = start + count;
        if (end >= buf->size) {
            end = buf->size;
        }
        int copy_size = (int)(end - start);

        // 写入数据
        net_err_t err = pktbuf_write(dest, buf->data + start, (int)copy_size);
        dbg_assert(err >= 0, "write buffer failed.");

        // 更新start，处理回绕的问题
        start += copy_size;
        if (start >= buf->size) {
            start -= buf->size;
        }
        count -= copy_size;

        // 不调整buf中的count和out，因为只当被确认时才需要
    }
}

int tcp_buf_remove (tcp_buf_t * buf, int cnt) {
    if (cnt > buf->count) {
        cnt = buf->count;
    }

    buf->out += cnt;
    if (buf->out >= buf->size) {
        buf->out -= buf->size;
    }

    buf->count -= cnt;
    return cnt;
}
