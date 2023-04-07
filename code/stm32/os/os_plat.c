
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "os_mem.h"
#include "os_mutex.h"
#include "os_plat.h"
#include "os_task.h"
#include "os_core.h"
#include "os_list.h"
#include "stm32f4xx.h" // Device header
#include "usart.h"
#include "ff.h"
#include <rt_sys.h>

os_task_t *task_from, *task_to;

// 在任务切换中，主要依赖了PendSV进行切换。PendSV其中的一个很重要的作用便是用于支持RTOS的任务切换。
// 实现方法为：
// 1、首先将PendSV的中断优先配置为最低。这样只有在其它所有中断完成后，才会触发该中断；
//    实现方法为：向NVIC_SYSPRI2写NVIC_PENDSV_PRI
// 2、在需要中断切换时，设置挂起位为1，手动触发。这样，当没有其它中断发生时，将会引发PendSV中断。
//    实现方法为：向NVIC_INT_CTRL写NVIC_PENDSVSET
// 3、在PendSV中，执行任务切换操作。
#define NVIC_INT_CTRL 0xE000ED04   // 中断控制及状态寄存器
#define NVIC_PENDSVSET 0x10000000  // 触发软件中断的值
#define NVIC_SYSPRI2 0xE000ED22    // 系统优先级寄存器
#define NVIC_PENDSV_PRI 0x000000FF // 配置优先级

#define MEM32(addr) *(volatile unsigned long *)(addr)
#define MEM8(addr) *(volatile unsigned char *)(addr)

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define IS_STD(fh) ((fh) >= 0 && (fh) <= 2)

const char __stdin_name[] = ":tt";
const char __stdout_name[] = ":tt";
const char __stderr_name[] = ":tt";

FILEHANDLE _sys_open(const char *name, int openmode) {
  if (name == __stdin_name) {
    return STDIN;
  } else if (name == __stdout_name) {
    uart_init(115200);
    return STDOUT;
  } else if (name == __stderr_name) {
    return STDERR;
  }

  FIL * fp = os_mem_alloc(sizeof(FIL));
  if (fp == NULL) {
    return -1;
  }

  /* http://elm-chan.org/fsw/ff/doc/open.html */
  BYTE mode;
  if (openmode & OPEN_W) {
      mode = FA_CREATE_ALWAYS | FA_WRITE;
      if (openmode & OPEN_PLUS) {
        mode |= FA_READ;
      }
  } else if (openmode & OPEN_A) {
    //mode = FA_OPEN_APPEND | FA_WRITE;
    mode = FA_WRITE;
    if (openmode & OPEN_PLUS) {
      mode |= FA_READ;
    }
  } else {
    mode = FA_READ;
    if (openmode & OPEN_PLUS) {
      mode |= FA_WRITE;
    }
  }

  FRESULT fr = f_open(fp, name, mode);
  if (fr == FR_OK) {
    return (uintptr_t)fp;
  }

  os_mem_free(fp);
  return -1;
}

int _sys_close(FILEHANDLE fh) { 
  if (IS_STD(fh)) {
    return 0;
  }
  
  FRESULT fr = f_close((FIL *)fh);
  if (fr == FR_OK) {
    os_mem_free((void *)fh);
    return 0;
  }
  
  return -1;
}

int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode) {
  if (fh == STDIN) {
    return -1;
  }
  
  if (fh == STDOUT || fh == STDERR) {
      for (int i = 0; i < len; i++) {
        usart_write(buf[i]);
      }
        return 0;
  }
 
  UINT bw;
  FRESULT fr = f_write((FIL *)fh, buf, len, &bw);
  if (fr == FR_OK) {
    return len - bw;
  }
  
  return -1;
}

int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode) {
  char ch;
  int i = 0;
  
  if (fh == STDIN) {
    while (i < len) {
      ch = usart_read();
      if (isprint(ch)) {
        buf[i++] = ch;
        usart_write(ch);
      } else if (ch == '\r') {
        buf[i++] = '\n';
        usart_write('\n');
        break;
      } else if (i > 0 && ch == '\b') {
        i--;
        usart_write('\b');
        usart_write(' ');
        usart_write('\b');
      }
    }

    return len - i;
  }
  
  if (fh == STDOUT || fh == STDERR) {
    return -1;
  }
  
  UINT br;
  FRESULT fr = f_read((FIL *)fh, buf, len, &br);
  if (fr == FR_OK) {
    return len - br;
  }

  return -1;
}

void _ttywrch(int ch) {
  usart_write(ch);
}

int _sys_istty(FILEHANDLE fh) {
    return IS_STD(fh);
}

int _sys_seek(FILEHANDLE fh, long pos) {
  FRESULT fr;
  
  if (!IS_STD(fh)) {
    fr = f_lseek((FIL *)fh, pos);
    if (fr == FR_OK)
      return 0;
  }
  return -1;
}

long _sys_flen(FILEHANDLE fh) {
  if (!IS_STD(fh)) {
    return f_size((FIL *)fh);
  }

  return -1;
}

// FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x) {
	  while (1) {}
}

os_mutex_t mutex_table[10];
int alloc_index;

// 用于库内部互斥的一个接口，临时使用，待修改
// https://developer.arm.com/documentation/dui0475/m/the-arm-c-and-c---libraries/multithreaded-support-in-arm-c-libraries/management-of-locks-in-multithreaded-applications
__attribute__((used)) int _mutex_initialize(os_mutex_t **sid) {

    os_mutex_t * mutex = mutex_table + alloc_index++;
    os_mutex_init(mutex);

    // 必须成功
    *sid = mutex;
    return 1;
}
__attribute__((used)) void _mutex_acquire(os_mutex_t **sid) {
    os_mutex_lock(*sid, 0);
}
__attribute__((used)) void _mutex_release(os_mutex_t **sid) {
    os_mutex_unlock(*sid);
}
__attribute__((used)) void _mutex_free(os_mutex_t **sid) {
    //os_mutex_free(*sid);
}

/**
 * 任务运行环境初始化
 * @param task 等待初始化的任务
 */
void os_task_ctx_init(os_task_t *task, void (*entry)(void *), void *param)
{
  // 以下的有些寄存器给出了默认的数值，方便在IDE中调试观察效果
  os_task_ctx_t *ctx = (os_task_ctx_t *)((uint32_t)task->start_stack + task->stack_size - sizeof(os_task_ctx_t));
  ctx->r0 = (cpu_stack_t)param; // 传给任务的入口函数
  ctx->r1 = 0x1;
  ctx->r2 = 0x2;
  ctx->r3 = 0x3;
  ctx->r4 = 0x4;
  ctx->r5 = 0x5;
  ctx->r6 = 0x6;
  ctx->r7 = 0x7;
  ctx->r8 = 0x8;
  ctx->r9 = 0x9;
  ctx->r10 = 0x10;
  ctx->r11 = 0x11;
  ctx->r12 = 0x12;
  ctx->r14_lr = (cpu_stack_t)os_task_exit; // 返回地址
  ctx->pr15_pc = (cpu_stack_t)entry;
  ctx->xpsr = (1 << 24); // 设置了Thumb模式，恢复到Thumb状态而非ARM状态运行

  // 记录一下
  task->ctx = ctx;
}

/**
 * 进入关中断保护状态
 */
os_protect_t os_enter_protect(void) {
  os_protect_t primask = __get_PRIMASK();
  __disable_irq(); // CPSID I
  return primask;
}

/**
 * 退出关中断保护状态
 */
void os_leave_protect(os_protect_t protect) {
  __set_PRIMASK(protect);
}

void os_task_switch_to(struct _os_task_t *to) {
    __set_PSP(0);
    os_leave_protect(0);
    MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET; // 向NVIC_INT_CTRL写NVIC_PENDSVSET，用于PendSV
}

/**
 * 从一个任务切换到另一个任务
 */
void os_task_switch(struct _os_task_t *from, struct _os_task_t *to) {
  MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;
}

void SysTick_Handler(void) {
  os_time_tick();
  os_sched_run();
}

void os_plat_init(void) {    
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  delay_init(168);
  LCD_Init(); 

  MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI; // 向NVIC_SYSPRI2写NVIC_PENDSV_PRI，设置其为最低优先级

  SystemCoreClockUpdate();
  SysTick->LOAD = OS_SYSTICK_MS * SystemCoreClock / 1000 - 1;
  NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}
