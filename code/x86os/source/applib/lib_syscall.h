/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "core/syscall.h"
#include "os_cfg.h"
#include "fs/file.h"
#include "dev/tty.h"

#include <sys/stat.h>
typedef struct _syscall_args_t {
    int id;
    int arg0;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
}syscall_args_t;

int msleep (int ms);
int fork(void);
int getpid(void);
int yield (void);
int execve(const char *name, char * const *argv, char * const *env);
int print_msg(char * fmt, int arg);
int wait(int* status);
void _exit(int status);

int open(const char *name, int flags, ...);
int read(int file, char *ptr, int len);
int write(int file, char *ptr, int len);
int close(int file);
int lseek(int file, int ptr, int dir);
int isatty(int file);
int fstat(int file, struct stat *st);
void * sbrk(ptrdiff_t incr);
int dup (int file);
int ioctl(int fd, int cmd, int arg0, int arg1);

struct dirent {
   int index;         // 在目录中的偏移
   int type;            // 文件或目录的类型
   char name [255];       // 目录或目录的名称
   int size;            // 文件大小
};

typedef struct _DIR {
    int index;               // 当前遍历的索引
    struct dirent dirent;
}DIR;

DIR * opendir(const char * name);
struct dirent* readdir(DIR* dir);
int closedir(DIR *dir);
int unlink(const char *pathname);

#include "sys.h"
#include "socket.h"
#include "tools.h"

#define in_addr         x_in_addr
#define sockaddr_in     x_sockaddr_in
#define sockaddr        x_sockaddr
#define socklen_t       x_socklen_t
#define timeval         x_timeval
#define hostent         x_hostent
#undef htons
#define htons(v)                x_htons(v)
#undef ntohs
#define ntohs(v)                x_ntohs(v)
#undef htonl
#define htonl(v)                x_htonl(v)
#undef ntohl
#define ntohl(v)                x_ntohl(v)
#define inet_ntoa(addr)             x_inet_ntoa(addr)
#define inet_addr(str)              x_inet_addr(str)
#define inet_pton(family, strptr, addrptr)          x_inet_pton(family, strptr, addrptr)
#define inet_ntop(family, addrptr, strptr, len)     x_inet_ntop(family, addrptr, strptr, len)

int socket(int family, int type, int protocol);
int closesocket(int sockfd);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct x_sockaddr* addr, x_socklen_t* len);
ssize_t sendto(int sid, const void* buf, size_t len, int flags, const struct x_sockaddr* dest, x_socklen_t dest_len);
ssize_t recvfrom(int sid, void* buf, size_t len, int flags, struct x_sockaddr* src, x_socklen_t* src_len);
int connect(int sid, const struct x_sockaddr* addr, x_socklen_t len);
int bind(int sid, const struct x_sockaddr* addr, x_socklen_t len);
ssize_t send(int fd, const void* buf, size_t len, int flags);
ssize_t recv(int fd, void* buf, size_t len, int flags);
int setsockopt(int sockfd, int level, int optname, const char * optval, int optlen);
int gethostbyname_r (const char *name,struct x_hostent *ret, char *buf, size_t buflen,
         struct x_hostent **result, int *h_errnop);

char* x_inet_ntoa(struct x_in_addr in);
in_addr_t x_inet_addr(const char* str);
int x_inet_pton(int family, const char *strptr, void *addrptr);
const char * x_inet_ntop(int family, const void *addrptr, char *strptr, size_t len);

#define inet_ntoa(addr)             x_inet_ntoa(addr)
#define inet_addr(str)              x_inet_addr(str)
#define inet_pton(family, strptr, addrptr)          x_inet_pton(family, strptr, addrptr)
#define inet_ntop(family, addrptr, strptr, len)     x_inet_ntop(family, addrptr, strptr, len)

#undef plat_printf
#define plat_printf    sprintf
#undef plat_strncpy
#define plat_strncpy    strncpy

#endif //LIB_SYSCALL_H
