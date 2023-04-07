#ifndef TCP_H
#define TCP_H

#include "sock.h"
#include "timer.h"
#include "exmsg.h"
#include "dbg.h"
#include "net_cfg.h"

#pragma pack(1)

/**
 * TCP连接块
 */
typedef struct _tcp_t {
    sock_t base;            // 基础sock结构
}tcp_t;

net_err_t tcp_init(void);

sock_t* tcp_create (int family, int protocol);

#endif // TCP_H

