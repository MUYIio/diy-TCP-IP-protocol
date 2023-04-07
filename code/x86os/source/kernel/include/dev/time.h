/**
 * 硬件定时器配置
 *
 * 创建时间：2022年6月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TIMER_H
#define TIMER_H

#include "comm/types.h"

#define PIT_OSC_FREQ                1193182				// 定时器时钟

// 定时器的寄存器和各项位配置
#define PIT_CHANNEL0_DATA_PORT       0x40
#define PIT_COMMAND_MODE_PORT        0x43

#define PIT_CHANNLE0                (0 << 6)
#define PIT_LOAD_LOHI               (3 << 4)
#define PIT_MODE0                   (3 << 1)

void time_init (void);
void exception_handler_timer (void);
uint32_t sys_get_ticks (void);

#endif //OS_TIMER_H
