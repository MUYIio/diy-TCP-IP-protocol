/**
 * 日志输出
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdarg.h>
#include "comm/cpu_instr.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "cpu/irq.h"
#include "os_cfg.h"
#include "ipc/mutex.h"
#include "dev/console.h"
#include "dev/dev.h"

// 目标用串口，参考资料：https://wiki.osdev.org/Serial_Ports
#define LOG_USE_COM         0
#define COM1_PORT           0x3F8       // RS232端口0初始化

static mutex_t mutex;
static int log_dev_id;

/**
 * @brief 初始化日志输出
 */
void log_init (void) {
    mutex_init(&mutex);

    log_dev_id = dev_open(DEV_TTY, 0, 0);

#if LOG_USE_COM
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
  
    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1_PORT + 4, 0x0F);
#endif
}

/**
 * @brief 日志打印
 */
void log_printf(const char * fmt, ...) {
    char str_buf[128];
    va_list args;

    kernel_memset(str_buf, '\0', sizeof(str_buf));

    va_start(args, fmt);
    kernel_vsprintf(str_buf, fmt, args);
    va_end(args);

    // 显示，如果发送速度太慢，会造成这里关中断太长时间
    // 所以，这里这样做不是好办法
    mutex_lock(&mutex);
    
#if LOG_USE_COM
    const char * p = str_buf;    
    while (*p != '\0') {
        while ((inb(COM1_PORT + 5) & (1 << 6)) == 0);
        outb(COM1_PORT, *p++);
    }

    outb(COM1_PORT, '\r');
    outb(COM1_PORT, '\n');
#else
    //console_write(0, str_buf, kernel_strlen(str_buf));
    dev_write(log_dev_id, 0, "log:", 4);
    dev_write(log_dev_id, 0, str_buf, kernel_strlen(str_buf));

    char c = '\n';
    //console_write(0, &c, 1);
    dev_write(log_dev_id, 0, &c, 1);

#endif
    mutex_unlock(&mutex);
}

