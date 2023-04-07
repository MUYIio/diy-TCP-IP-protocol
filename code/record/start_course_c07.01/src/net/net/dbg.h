#ifndef DBG_H
#define DBG_H

#define DBG_STYLE_ERROR  	"\033[31m"
#define DBG_STYLE_WARNING  	"\033[33m"
#define DBG_STYLE_RESET  	"\033[0m"

#define DBG_LEVEL_NONE      0
#define DBG_LEVEL_ERROR     1
#define DBG_LEVEL_WARNING   2
#define DBG_LEVEL_INFO      3

void dbg_print(int m_level, int s_level, const char * file, const char * func, int len, const char * fmt, ...);

#define dbg_info(module, fmt, ...)  dbg_print(module, DBG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_warning(module, fmt, ...)  dbg_print(module, DBG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_error(module, fmt, ...)  dbg_print(module, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

// dbg_assert(3 == 0, "error");
// assert failed: 3==0, error
#define dbg_assert(expr, msg)   {\
    if (!(expr)) {\
        dbg_print(DBG_LEVEL_ERROR, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, "assert failed:"#expr","msg);   \
        while (1);  \
    }\
}
#endif 