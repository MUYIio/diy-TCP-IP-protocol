#ifndef OS_DBG_H
#define OS_DBG_H

#include "os_cfg.h"

#if OS_DEBUG_ENABLE
/**
 * 打印调试信息
*/
#define os_dbg(fmt, ...)   //{\
    //printf("%s(%d):", __FUNCTION__, __LINE__);  \
    //printf(fmt, ##__VA_ARGS__); }

/**
 * os断言，当表达式不成立时，整个系统死机
 */
#define os_assert(expr)     {\
    if (!(expr))       {\
        os_enter_protect();	\
        os_dbg(#expr);    \
        while(1);\
    }   \
}

#else
#define os_dbg(fmt)        
#define os_assert(expr)
#endif // OS_DEBUG_ENABLE

#if OS_CHECK_PARAM == 1
#define os_param_failed(expr, ret)  {\
    if (expr)   return (ret);   \
}
#define os_param_failed_exec(expr, ret, exec)  {\
    if (expr)   {{exec;}; return (ret);};   \
}
#else 
#define os_param_failed(expr, ret)
#define os_param_failed_exec(expr, ret, exec)
#endif

#endif // OS_DBG_H
