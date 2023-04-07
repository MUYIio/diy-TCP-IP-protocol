#include "dbg.h"
#include "sys_plat.h"
#include <stdarg.h>

// d:/xxx/xxx/main.c
// main.c
// /home/xxx/main.c
void dbg_print(const char * file, const char * func, int line, const char * fmt, ...) {
    const char * end = file + plat_strlen(file);
    while (end >= file) {
        if ((*end == '\\') || (*end == '/')) {
            break;
        } 

        end--;
    }
    end++;

    plat_printf("(%s-%s-%d):", end, func, line);

    char str_buf[128];
    va_list args;

    va_start(args, fmt);
    plat_vsprintf(str_buf, fmt, args);
    plat_printf("%s\n", str_buf);
    va_end(args);
}
