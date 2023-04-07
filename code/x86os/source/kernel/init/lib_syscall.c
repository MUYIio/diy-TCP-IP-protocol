/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "os_cfg.h"
#include "applib/lib_syscall.h"
#include "malloc.h"

/**
 * 执行系统调用
 */
static inline int sys_call (syscall_args_t * args) {
    const uint32_t sys_gate_addr[] = {0, SELECTOR_SYSCALL | 0};  // 使用特权级0
    int ret;

    // 采用调用门, 这里只支持5个参数
    // 用调用门的好处是会自动将参数复制到内核栈中，这样内核代码很好取参数
    // 而如果采用寄存器传递，取参比较困难，需要先压栈再取
    __asm__ __volatile__ (
            "mov %[args_base], %%eax\r\n"
            "push 24(%%eax)\n\t"             // arg5
            "push 20(%%eax)\n\t"             // arg4
            "push 16(%%eax)\n\t"             // arg3
            "push 12(%%eax)\n\t"             // arg2
            "push 8(%%eax)\n\t"              // arg1
            "push 4(%%eax)\n\t"              // arg0
            "push 0(%%eax)\n\t"              // id
            "lcalll *(%[gate])\n\n"
            :"=a"(ret)
            :[args_base]"r"(args), [gate]"r"(sys_gate_addr));
    return ret;
}

int msleep (int ms) {
    if (ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.id = SYS_msleep;
    args.arg0 = ms;
    return sys_call(&args);
}

int getpid() {
    syscall_args_t args;
    args.id = SYS_getpid;
    return sys_call(&args);
}

int print_msg(char * fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_printmsg;
    args.arg0 = (int)fmt;
    args.arg1 = arg;
    return sys_call(&args);
}

int fork() {
    syscall_args_t args;
    args.id = SYS_fork;
    return sys_call(&args);
}

int execve(const char *name, char * const *argv, char * const *env) {
    syscall_args_t args;
    args.id = SYS_execve;
    args.arg0 = (int)name;
    args.arg1 = (int)argv;
    args.arg2 = (int)env;
    return sys_call(&args);
}

int yield (void) {
    syscall_args_t args;
    args.id = SYS_yield;
    return sys_call(&args);
}

int wait(int* status) {
    syscall_args_t args;
    args.id = SYS_wait;
    args.arg0 = (int)status;
    return sys_call(&args);
}

void _exit(int status) {
    syscall_args_t args;
    args.id = SYS_exit;
    args.arg0 = (int)status;
    sys_call(&args);
    for (;;) {}
}

int open(const char *name, int flags, ...) {
    // 不考虑支持太多参数
    syscall_args_t args;
    args.id = SYS_open;
    args.arg0 = (int)name;
    args.arg1 = (int)flags;
    return sys_call(&args);
}

int read(int file, char *ptr, int len) {
    syscall_args_t args;
    args.id = SYS_read;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = len;
    return sys_call(&args);
}

int write(int file, char *ptr, int len) {
    syscall_args_t args;
    args.id = SYS_write;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = len;
    return sys_call(&args);
}

int close(int file) {
    syscall_args_t args;
    args.id = SYS_close;
    args.arg0 = (int)file;
    return sys_call(&args);
}

int lseek(int file, int ptr, int dir) {
    syscall_args_t args;
    args.id = SYS_lseek;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = dir;
    return sys_call(&args);
}

/**
 * 获取文件的状态
 */
int fstat(int file, struct stat *st) {
    syscall_args_t args;
    args.id = SYS_fstat;
    args.arg0 = (int)file;
    args.arg1 = (int)st;
    return sys_call(&args);
}

/**
 * 判断文件描述符与tty关联
 */
int isatty(int file) {
    syscall_args_t args;
    args.id = SYS_isatty;
    args.arg0 = (int)file;
    return sys_call(&args);
}

void * sbrk(ptrdiff_t incr) {
    syscall_args_t args;
    args.id = SYS_sbrk;
    args.arg0 = (int)incr;
    return (void *)sys_call(&args);
}

int dup (int file) {
    syscall_args_t args;
    args.id = SYS_dup;
    args.arg0 = file;
    return sys_call(&args);
}
