#ifndef NET_ERR_H
#define NET_ERR_H

typedef enum _net_err_t {
    NET_ERR_OK = 0,
    NET_ERR_SYS = -1,
    NET_ERR_MEM = -2,
    NET_ERR_FULL = -3,
    NET_ERR_TMO = -4,
}net_err_t;

#endif 