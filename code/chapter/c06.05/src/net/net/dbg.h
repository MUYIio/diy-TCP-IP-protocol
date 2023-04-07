﻿/**
 * @brief 调试信息输出
 * 此处运行了一个核心线程，所有TCP/IP中相关的事件都交由该线程处理
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#ifndef DBG_H
#define DBG_H

// 调试信息的显示样式设置
#define DBG_STYLE_RESET       "\033[0m"       // 复位显示
#define DBG_STYLE_ERROR       "\033[31m"      // 红色显示
#define DBG_STYLE_WARNING     "\033[33m"      // 黄色显示

// 开启的信息输出配置，值越大，输出的调试信息越多
#define DBG_LEVEL_NONE           0         // 不开启任何输出
#define DBG_LEVEL_ERROR          1         // 只开启错误信息输出
#define DBG_LEVEL_WARNING        2         // 开启错误和警告信息输出
#define DBG_LEVEL_INFO           3         // 开启错误、警告、一般信息输出

/**
 * @brief 打印调试信息
 */
void dbg_print(int m_level, int s_level, const char* file, const char* func, int line, const char* fmt, ...);

/**
 * @brief 不同的调试输出宏
 * __FILE__ __FUNCTION__, __LINE__为C语言内置的宏
*/
#define dbg_info(module, fmt, ...)  dbg_print(module, DBG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_error(module, fmt, ...)  dbg_print(module, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define dbg_warning(module, fmt, ...) dbg_print(module, DBG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief 断言判断
 * 下面用大括号包含，这样避免if可能与后面可能的else相边，例如：
 * if (xxx)
 *    dbg_assert   多个了if (xxx) 
 * else
 *    xxxx
 * 不过一般我会用大括号包含if下面的语句，所以出错可能性不大
 */
#define dbg_assert(expr, msg)   {\
    if (!(expr)) {\
        dbg_print(DBG_LEVEL_ERROR, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, "assert failed:"#expr","msg); \
        while(1);   \
    }   \
}
#endif  // DBG_H

