/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "os_cfg.h"
#include "lib_syscall.h"
#include "malloc.h"
#include <string.h>

/**
 * 执行系统调用
 */
static int sys_call (syscall_args_t * args) {
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

int ioctl(int fd, int cmd, int arg0, int arg1) {
    syscall_args_t args;
    args.id = SYS_ioctl;
    args.arg0 = fd;
    args.arg1 = cmd;
    args.arg2 = arg0;
    args.arg3 = arg1;
    return sys_call(&args);
}

DIR * opendir(const char * name) {
    DIR * dir = (DIR *)malloc(sizeof(DIR));
    if (dir == (DIR *)0) {
        return (DIR *)0;
    }

    syscall_args_t args;
    args.id = SYS_opendir;
    args.arg0 = (int)name;
    args.arg1 = (int)dir;
    int err = sys_call(&args);
    if (err < 0) {
        free(dir);
        return (DIR *)0;
    }
    return dir;
}

struct dirent* readdir(DIR* dir) {

    syscall_args_t args;
    args.id = SYS_readdir;
    args.arg0 = (int)dir;
    args.arg1 = (int)&dir->dirent;
    int err = sys_call(&args);
    if (err < 0) {
        return (struct dirent *)0;
    }
    return &dir->dirent;
}

int closedir(DIR *dir) {
    syscall_args_t args;
    args.id = SYS_closedir;
    args.arg0 = (int)dir;
    sys_call(&args);

    free(dir);
    return 0;
}

int unlink(const char *path) {
    syscall_args_t args;
    args.id = SYS_unlink;
    args.arg0 = (int)path;
    return sys_call(&args);
}

int socket(int family, int type, int protocol) {
    syscall_args_t args;
    args.id = SYS_socket;
    args.arg0 = (int)family;
    args.arg1 = (int)type;
    args.arg2 = (int)protocol;
    return sys_call(&args);
}

int closesocket(int sockfd) {
    syscall_args_t args;
    args.id = SYS_closesocket;
    args.arg0 = (int)sockfd;
    return sys_call(&args);
}

int listen(int sockfd, int backlog) {
    syscall_args_t args;
    args.id = SYS_listen;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)backlog;
    return sys_call(&args);
}

int accept(int sockfd, struct x_sockaddr* addr, x_socklen_t* len) {
    syscall_args_t args;
    args.id = SYS_accept;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)addr;
    args.arg2 = (int)len;
    return sys_call(&args);
}

ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct x_sockaddr* dest, x_socklen_t dest_len) {
    syscall_args_t args;
    args.id = SYS_sendto;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)buf;
    args.arg2 = (int)len;
    args.arg3 = (int)flags;
    args.arg4 = (int)dest;
    args.arg5 = (int)dest_len;
    return (ssize_t)sys_call(&args);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct x_sockaddr* src, x_socklen_t* src_len) {
    syscall_args_t args;
    args.id = SYS_recvfrom;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)buf;
    args.arg2 = (int)len;
    args.arg3 = (int)flags;
    args.arg4 = (int)src;
    args.arg5 = (int)src_len;
    return (ssize_t)sys_call(&args);    
}

int connect(int sockfd, const struct x_sockaddr* addr, x_socklen_t len) {
    syscall_args_t args;
    args.id = SYS_connect;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)addr;
    args.arg2 = (int)len;
    return (ssize_t)sys_call(&args); 
}

int bind(int sockfd, const struct x_sockaddr* addr, x_socklen_t len) {
    syscall_args_t args;
    args.id = SYS_bind;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)addr;
    args.arg2 = (int)len;
    return (ssize_t)sys_call(&args); 
}

ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
    syscall_args_t args;
    args.id = SYS_send;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)buf;
    args.arg2 = (int)len;
    args.arg3 = (int)flags;
    return (ssize_t)sys_call(&args); 
}

ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
    syscall_args_t args;
    args.id = SYS_recv;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)buf;
    args.arg2 = (int)len;
    args.arg3 = (int)flags;
    return (ssize_t)sys_call(&args); 
}

int setsockopt(int sockfd, int level, int optname, const char * optval, int optlen) {
    syscall_args_t args;
    args.id = SYS_setsockopt;
    args.arg0 = (int)sockfd;
    args.arg1 = (int)level;
    args.arg2 = (int)optname;
    args.arg3 = (int)optval;
    args.arg4 = (int)optlen;
    return sys_call(&args); 
}

int gethostbyname_r (const char *name,struct x_hostent *ret, char *buf, size_t buflen,
         struct x_hostent **result, int *h_errnop) {
    syscall_args_t args;
    args.id = SYS_gethostbyname_r;
    args.arg0 = (int)name;
    args.arg1 = (int)ret;
    args.arg2 = (int)buf;
    args.arg3 = (int)buflen;
    args.arg4 = (int)result;
    args.arg5 = (int)h_errnop;
    return sys_call(&args); 
}


#define IPV4_STR_SIZE           16          // IPv4地址的字符串长

/**
 * @brief IP地址转换为字符串
 * 之所以放在这里，是使得调用方都有自己的副本，不会因内部的静态数据而导致相应冲突
 */
char* x_inet_ntoa(struct x_in_addr in) {
    static char buf[IPV4_STR_SIZE];
    sprintf(buf, "%d.%d.%d.%d", in.addr0, in.addr1, in.addr2, in.addr3);
    return buf;
}

/**
 * @brief 将ip地址转换为32位的无符号数
 */
in_addr_t x_inet_addr(const char* str) {
    if (!str) {
        return INADDR_ANY;
    }

    // 初始值清空
    in_addr_t addr = 0;

    // 不断扫描各字节串，直到字符串结束
    char c;
    uint8_t * p = (uint8_t *)&addr;
    uint8_t sub_addr = 0;
    while ((c = *str++) != '\0') {
        // 如果是数字字符，转换成数字并合并进去
        if ((c >= '0') && (c <= '9')) {
            // 数字字符转换为数字，再并入计算
            sub_addr = sub_addr * 10 + c - '0';
        } else if (c == '.') {
            // . 分隔符，进入下一个地址符，并重新计算
            *p++ = sub_addr;
            sub_addr = 0;
        } else {
            // 格式错误，包含非数字和.字符
            return NET_ERR_PARAM;
        }
    }

    // 写入最后计算的值
    *p++ = sub_addr;
    return addr;    
}

/**
 * @brief 将点分十进制的IP地址转换为网络地址
 * 若成功则为1，若输入不是有效的表达式则为0，若出错则为-1
 */
int x_inet_pton(int family, const char *strptr, void *addrptr) {
    // 仅支持IPv4地址类型
    if ((family != AF_INET) || !strptr || !addrptr) {
        return -1;
    }
    struct x_in_addr * addr = (struct x_in_addr *)addrptr;
    addr->s_addr = x_inet_addr(strptr);
    return 1;
}

/**
 * @brief 将网络地址转换为字符串形式的地址
 */
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len) {
    if ((family != AF_INET) || !addrptr || !strptr || !len) {
        return (const char *)0;
    }

    struct x_in_addr * addr = (struct x_in_addr *)addrptr;
    char buf[IPV4_STR_SIZE];
    sprintf(buf, "%d.%d.%d.%d", addr->addr0, addr->addr1, addr->addr2, addr->addr3);
    plat_strncpy(strptr, buf, len - 1);
    strptr[len - 1] = '\0';
    return strptr;
}
