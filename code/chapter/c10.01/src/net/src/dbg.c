/**
 * @brief 调试信息输出
 * 此处运行了一个核心线程，所有TCP/IP中相关的事件都交由该线程处理
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note
 */
#include <string.h>
#include <stdarg.h>
#include "dbg.h"
#include "net_plat.h"

/**
 * @brief 调试信息输出
 * 输出文件名、函数名、函数以及调试信息
 */
void dbg_print(int m_level, int s_level, const char* file, const char* func, int line, const char* fmt, ...) {
    static const char* title[] = {
        [DBG_LEVEL_ERROR] = DBG_STYLE_ERROR"error",
        [DBG_LEVEL_WARNING] = DBG_STYLE_WARNING"warning",
        [DBG_LEVEL_INFO] = "info",
        [DBG_LEVEL_NONE] = "none"
    };

    // 当仅当前等级数值比较大时才输出
    if (m_level >= s_level) {
        // 定位到文件名部分，因为file传输的是完整的文件路径，太长了
        const char * end = file + plat_strlen(file);
        while (end >= file) {
            if ((*end == '\\') || (*end == '/')) {
                break;
            }
            end--;
        }
        end++;

        // 每行信息提示的开头
        plat_printf("%s(%s-%s-%d):", title[s_level], end, func, line);

        char str_buf[128];
        va_list args;

        // 具体的信息
        va_start(args, fmt);
        plat_vsprintf(str_buf, fmt, args);
        plat_printf("%s\n"DBG_STYLE_RESET, str_buf);
        va_end(args);
    }
}


