/**
 * 一些字符串的处理函数
 *
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "tools/klib.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"

/**
 * @brief 计算字符串的数量
 */
int strings_count (char ** start) {
    int count = 0;

    if (start) {
        while (*start++) {
            count++;
        }
    }
    return count;
}


/**
 * @brief 从路径中解释文件名
 */
char * get_file_name (char * name) {
    char * s = name;

    // 定位到结束符
    while (*s != '\0') {
        s++;
    }

    // 反向搜索，直到找到反斜杆或者到文件开头
    while ((*s != '\\') && (*s != '/') && (s >= name)) {
        s--;
    }
    return s + 1;
}

void kernel_strcpy (char * dest, const char * src) {
    if (!dest || !src) {
        return;
    }

    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void kernel_strncpy(char * dest, const char * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    char * d = dest;
    const char * s = src;

    while ((size-- > 0) && (*s)) {
        *d++ = *s++;
    }
    if (size == 0) {
        *(d - 1) = '\0';
    } else {
        *d = '\0';
    }
}

int kernel_strlen(const char * str) {
    if (str == (const char *)0) {
        return 0;
    }

	const char * c = str;

	int len = 0;
	while (*c++) {
		len++;
	}

	return len;
}

/**
 * 比较两个字符串，最多比较size个字符
 * 如果某一字符串提前比较完成，也算相同
 */
int kernel_strncmp (const char * s1, const char * s2, int size) {
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (*s1 == *s2) && size) {
    	s1++;
    	s2++;
    }

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}

int kernel_strcmp (const char * s1, const char * s2) {
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (*s1 == *s2)) {
    	s1++;
    	s2++;
    }

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}

char kernel_tolower (char ch) {
    if ((ch >= 'A') && (ch <= 'Z')) {
        return ch - 'A' + 'a';
    } else {
        return ch;
    }
}

int kernel_stricmp (const char * s1, const char * s2) {
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (kernel_tolower(*s1) == kernel_tolower(*s2))) {
    	s1++;
    	s2++;
    }

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}

void kernel_memcpy (void * dest, const void * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    const uint8_t * s = (const uint8_t *)src;
    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void * dest, uint8_t v, int size) {
    if (!dest || !size) {
        return;
    }

    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = v;
    }
}

int kernel_memcmp (const void * d1, const void * d2, int size) {
    if (!d1 || !d2) {
        return 1;
    }

	const uint8_t * p_d1 = (uint8_t *)d1;
	const uint8_t * p_d2 = (uint8_t *)d2;
	while (size--) {
		if (*p_d1++ != *p_d2++) {
			return 1;
		}
	}

	return 0;
}

void kernel_itoa(char * buf, int num, int base) {
    // 转换字符索引[-15, -14, ...-1, 0, 1, ...., 14, 15]
    static const char * num2ch = {"FEDCBA9876543210123456789ABCDEF"};
    char * p = buf;
    int old_num = num;

    // 仅支持部分进制
    if ((base != 2) && (base != 8) && (base != 10) && (base != 16)) {
        *p = '\0';
        return;
    }

    // 只支持十进制负数
    int signed_num = 0;
    if ((num < 0) && (base == 10)) {
        *p++ = '-';
        signed_num = 1;
    }

    if (signed_num) {
        do {
            char ch = num2ch[num % base + 15];
            *p++ = ch;
            num /= base;
        } while (num);
    } else {
        uint32_t u_num = (uint32_t)num;
        do {
            char ch = num2ch[u_num % base + 15];
            *p++ = ch;
            u_num /= base;
        } while (u_num);
    }
    *p-- = '\0';

    // 将转换结果逆序，生成最终的结果
    char * start = (!signed_num) ? buf : buf + 1;
    while (start < p) {
        char ch = *start;
        *start = *p;
        *p-- = ch;
        start++;
    }
}

/**
 * @brief 格式化字符串到缓存中
 */
void kernel_sprintf(char * buffer, const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kernel_vsprintf(buffer, fmt, args);
    va_end(args);
}

/**
 * 格式化字符串
 */
void kernel_vsprintf(char * buffer, const char * fmt, va_list args) {
    enum {NORMAL, READ_FMT} state = NORMAL;
    char ch;
    char * curr = buffer;
    while ((ch = *fmt++)) {
        switch (state) {
            // 普通字符
            case NORMAL:
                if (ch == '%') {
                    state = READ_FMT;
                } else {
                    *curr++ = ch;
                }
                break;
            // 格式化控制字符，只支持部分
            case READ_FMT:
                if (ch == 'd') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 10);
                    curr += kernel_strlen(curr);
                } else if (ch == 'x') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);
                } else if (ch == 'c') {
                    char c = va_arg(args, int);
                    *curr++ = c;
                } else if (ch == 's') {
                    const char * str = va_arg(args, char *);
                    int len = kernel_strlen(str);
                    while (len--) {
                        *curr++ = *str++;
                    }
                }
                state = NORMAL;
                break;
        }
    }
}

void panic (const char * file, int line, const char * func, const char * cond) {
    log_printf("assert failed! %s", cond);
    log_printf("file: %s\nline %d\nfunc: %s\n", file, line, func);

    for (;;) {
        hlt();
    }
}


