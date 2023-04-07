/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 
 * 只支持VGA模式
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "comm/types.h"
#include "dev/tty.h"
#include "ipc/mutex.h"

// https://wiki.osdev.org/Printing_To_Screen
#define CONSOLE_VIDEO_BASE			0xb8000		// 控制台显存起始地址,共32KB
#define CONSOLE_DISP_ADDR           0xb8000
#define CONSOLE_DISP_END			(0xb8000 + 32*1024)	// 显存的结束地址
#define CONSOLE_ROW_MAX				25			// 行数
#define CONSOLE_COL_MAX				80			// 最大列数

#define ASCII_ESC                   0x1b        // ESC ascii码            

#define	ESC_PARAM_MAX				10			// 最多支持的ESC [ 参数数量

// 各种颜色
typedef enum _cclor_t {
    COLOR_Black			= 0,
    COLOR_Blue			= 1,
    COLOR_Green			= 2,
    COLOR_Cyan			= 3,
    COLOR_Red			= 4,
    COLOR_Magenta		= 5,
    COLOR_Brown			= 6,
    COLOR_Gray			= 7,
    COLOR_Dark_Gray 	= 8,
    COLOR_Light_Blue	= 9,
    COLOR_Light_Green	= 10,
    COLOR_Light_Cyan	= 11,
    COLOR_Light_Red		= 12,
    COLOR_Light_Magenta	= 13,
    COLOR_Yellow		= 14,
    COLOR_White			= 15
}cclor_t;

/**
 * @brief 显示字符
 */
typedef union {
	struct {
		char c;						// 显示的字符
		char foreground : 4;		// 前景色
		char background : 3;		// 背景色
	};

	uint16_t v;
}disp_char_t;

/**
 * 终端显示部件
 */
typedef struct _console_t {
	disp_char_t * disp_base;	// 显示基地址

    enum {
        CONSOLE_WRITE_NORMAL,			// 普通模式
        CONSOLE_WRITE_ESC,				// ESC转义序列
        CONSOLE_WRITE_SQUARE,           // ESC [接收状态
    }write_state;

    int cursor_row, cursor_col;		// 当前编辑的行和列
    int display_rows, display_cols;	// 显示界面的行数和列数
    int old_cursor_col, old_cursor_row;	// 保存的光标位置
    cclor_t foreground, background;	// 前后景色

    int esc_param[ESC_PARAM_MAX];	// ESC [ ;;参数数量
    int curr_param_index;

    mutex_t mutex;                  // 写互斥锁
}console_t;

int console_init (int idx);
int console_write (tty_t * tty);
void console_close (int dev);
void console_select(int idx);
void console_set_cursor(int idx, int visiable);
#endif /* SRC_UI_TTY_WIDGET_H_ */
