/**
 * 系统调用实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "tools/klib.h"
#include "core/task.h"
#include "tools/log.h"
#include "core/memory.h"
#include "fs/fs.h"

#include "socket.h"

// 系统调用处理函数类型
typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

int sys_print_msg (char * fmt, int arg) {
	log_printf(fmt, arg);
}

// 系统调用表
static const syscall_handler_t sys_table[] = {
	[SYS_msleep] = (syscall_handler_t)sys_msleep,
    [SYS_getpid] =(syscall_handler_t)sys_getpid,

    [SYS_printmsg] = (syscall_handler_t)sys_print_msg,
	[SYS_fork] = (syscall_handler_t)sys_fork,
	[SYS_execve] = (syscall_handler_t)sys_execve,
    [SYS_yield] = (syscall_handler_t)sys_yield,
	[SYS_wait] = (syscall_handler_t)sys_wait,
	[SYS_exit] = (syscall_handler_t)sys_exit,

	[SYS_open] = (syscall_handler_t)sys_open,
	[SYS_read] = (syscall_handler_t)sys_read,
	[SYS_write] = (syscall_handler_t)sys_write,
	[SYS_close] = (syscall_handler_t)sys_close,
	[SYS_lseek] = (syscall_handler_t)sys_lseek,
	[SYS_isatty] = (syscall_handler_t)sys_isatty,
	[SYS_sbrk] = (syscall_handler_t)sys_sbrk,
	[SYS_fstat] = (syscall_handler_t)sys_fstat,
	[SYS_dup] = (syscall_handler_t)sys_dup,
	[SYS_ioctl] = (syscall_handler_t)sys_ioctl,

    [SYS_opendir] = (syscall_handler_t)sys_opendir,
	[SYS_readdir] = (syscall_handler_t)sys_readdir,
	[SYS_closedir] = (syscall_handler_t)sys_closedir,
	[SYS_unlink] = (syscall_handler_t)sys_unlink,

	[SYS_socket] = (syscall_handler_t)x_socket,	
	[SYS_listen] = (syscall_handler_t)x_listen,			
	[SYS_accept] = (syscall_handler_t)x_accept,
	[SYS_sendto] = (syscall_handler_t)x_sendto,
	[SYS_recvfrom] = (syscall_handler_t)x_recvfrom,
	[SYS_connect] = (syscall_handler_t)x_connect,		
	[SYS_bind] = (syscall_handler_t)x_bind,	
	[SYS_send] = (syscall_handler_t)x_send,
	[SYS_recv] = (syscall_handler_t)x_recv,
	[SYS_setsockopt] = (syscall_handler_t)x_setsockopt,
	[SYS_gethostbyname_r] = (syscall_handler_t)x_gethostbyname_r,
	[SYS_closesocket] = (syscall_handler_t)x_close,
};

/**
 * 处理系统调用。该函数由系统调用函数调用
 */
void do_handler_syscall (syscall_frame_t * frame) {
	// 超出边界，返回错误
    if (frame->func_id < sizeof(sys_table) / sizeof(sys_table[0])) {
		// 查表取得处理函数，然后调用处理
		syscall_handler_t handler = sys_table[frame->func_id];
		if (handler) {
			int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3, frame->arg4, frame->arg5);
			frame->eax = ret;  // 设置系统调用的返回值，由eax传递
            return;
		}
	}

	// 不支持的系统调用，打印出错信息
	task_t * task = task_current();
	log_printf("task: %s, Unknown syscall: %d", task->name,  frame->func_id);
    frame->eax = -1;  // 设置系统调用的返回值，由eax传递
}
