/**
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

/**
 * @brief 打印调试信息
 */
void dbg_print(const char* file, const char* func, int line, const char* fmt, ...);

/**
 * @brief 不同的调试输出宏
 * __FILE__ __FUNCTION__, __LINE__为C语言内置的宏
*/ 
#define dbg_info(fmt, ...)  dbg_print(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif  // DBG_H

