#ifndef OS_ERR_H
#define OS_ERR_H

typedef enum _os_err_t {
    OS_ERR_OK = 0,
    OS_ERR_PARAM = -1,
    OS_ERR_NONE = -2,
    OS_ERR_REMOVE = -3,
    OS_ERR_LOCKED = -4,
    OS_ERR_UNLOCKED = -5,
    OS_ERR_OWNER = -6,
    OS_ERR_TMO = -7,
    OS_ERR_STATE = -8,
    OS_ERR_FULL = -9,
}os_err_t;

#endif // OS_ERR_H
