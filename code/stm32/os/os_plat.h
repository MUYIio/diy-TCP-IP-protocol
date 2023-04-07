#ifndef OS_PLAT_H
#define OS_PLAT_H

#include <stdint.h>
#include <stdio.h>

typedef uint32_t os_protect_t;
typedef uint32_t cpu_stack_t;

typedef struct _os_task_ctx_t {
    cpu_stack_t r4;
    cpu_stack_t r5;
    cpu_stack_t r6;
    cpu_stack_t r7;
    cpu_stack_t r8;
    cpu_stack_t r9;
    cpu_stack_t r10;
    cpu_stack_t r11;
    cpu_stack_t r0;
    cpu_stack_t r1;
    cpu_stack_t r2;
    cpu_stack_t r3;
    cpu_stack_t r12;
    cpu_stack_t r14_lr;
    cpu_stack_t pr15_pc;
    cpu_stack_t xpsr;
}os_task_ctx_t;

void os_plat_init (void);

struct _os_task_t;

os_protect_t os_enter_protect (void);
void os_leave_protect (os_protect_t protect);

void os_task_switch_to (struct _os_task_t * to);
void os_task_switch(struct _os_task_t * from, struct _os_task_t * to);
void os_task_ctx_init (struct _os_task_t * task,  void (*entry)(void *), void *param);

#endif // OS_PLAT_H
