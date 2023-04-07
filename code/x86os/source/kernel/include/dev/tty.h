/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TTY_H
#define TTY_H

#include "ipc/sem.h"

#define TTY_NR						1		// 最大支持的tty设备数量
#define TTY_IBUF_SIZE				512		// tty输入缓存大小
#define TTY_OBUF_SIZE				512		// tty输出缓存大小
#define TTY_CMD_ECHO				0x1		// 开回显
#define TTY_CMD_IN_COUNT			0x2		// 获取输入缓冲区中已有的数据量

typedef struct _tty_fifo_t {
	char * buf;
	int size;				// 最大字节数
	int read, write;		// 当前读写位置
	int count;				// 当前已有的数据量
}tty_fifo_t;

int tty_fifo_get (tty_fifo_t * fifo, char * c);
int tty_fifo_put (tty_fifo_t * fifo, char c);

#define TTY_INLCR			(1 << 0)		// 将\n转成\r\n
#define TTY_IECHO			(1 << 2)		// 是否回显

#define TTY_OCRLF			(1 << 0)		// 输出是否将\n转换成\r\n

/**
 * tty设备
 */
typedef struct _tty_t {
	char obuf[TTY_OBUF_SIZE];
	tty_fifo_t ofifo;				// 输出队列
	sem_t osem;
	char ibuf[TTY_IBUF_SIZE];
	tty_fifo_t ififo;				// 输入处理后的队列
	sem_t isem;

	int iflags;						// 输入标志
    int oflags;						// 输出标志
	int console_idx;				// 控制台索引号
}tty_t;

void tty_select (int tty);
void tty_in (char ch);

#endif /* TTY_H */
