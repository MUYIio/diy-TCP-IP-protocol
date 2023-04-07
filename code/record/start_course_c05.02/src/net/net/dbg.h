#ifndef DBG_H
#define DBG_H

#define DBG_STYLE_ERROR  	"\033[31m"
#define DBG_STYLE_WARNING  	"\033[33m"
#define DBG_STYLE_RESET  	"\033[0m"

void dbg_print(const char * file, const char * func, int len, const char * fmt, ...);

#define dbg_info(fmt, ...)  dbg_print(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif 