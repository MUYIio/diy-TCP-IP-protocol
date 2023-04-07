/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 参考资料：https://wiki.osdev.org/Printing_To_Screen
 */
#include "dev/console.h"
#include "tools/klib.h"
#include "comm/cpu_instr.h"
#include "dev/tty.h"
#include "cpu/irq.h"

#define CONSOLE_NR          8           // 控制台的数量

static console_t console_buf[CONSOLE_NR];

/**
 * @brief 读取当前光标的位置
 */
static int read_cursor_pos (void) {
    int pos;

    irq_state_t state = irq_enter_protection();
 	outb(0x3D4, 0x0F);		// 写低地址
	pos = inb(0x3D5);
	outb(0x3D4, 0x0E);		// 写高地址
	pos |= inb(0x3D5) << 8;   
    irq_leave_protection(state);
    return pos;
}

/**
 * @brief 更新鼠标的位置
 */
static void update_cursor_pos (console_t * console) {
	uint16_t pos = (console - console_buf) * (console->display_cols * console->display_rows);
    pos += console->cursor_row *  console->display_cols + console->cursor_col;

    irq_state_t state = irq_enter_protection();
	outb(0x3D4, 0x0F);		// 写低地址
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);		// 写高地址
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
    irq_leave_protection(state);
}

void console_set_cursor(int idx, int visiable) {
    console_t *console = console_buf + idx;

    irq_state_t state = irq_enter_protection();
    if (visiable) {
        outb(0x3D4, 0x0A);
        outb(0x3D5, (inb(0x3D5) & 0xC0) | 0);
        outb(0x3D4, 0x0B);
        outb(0x3D5, (inb(0x3D5) & 0xE0) | 15);
    } else {
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
    }
    irq_leave_protection(state);
}


void console_select(int idx) {
    console_t * console = console_buf + idx;
    if (console->disp_base == 0) {
        // 可能没有初始化，先初始化一下
        console_init(idx);
    }

	uint16_t pos = idx * console->display_cols * console->display_rows;

	outb(0x3D4, 0xC);		// 写高地址
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
	outb(0x3D4, 0xD);		// 写低地址
	outb(0x3D5, (uint8_t) (pos & 0xFF));

    // 更新光标到当前屏幕
    update_cursor_pos(console);
}
/**
 * @brief 擦除从start到end的行
 */
static void erase_rows (console_t * console, int start, int end) {
    volatile disp_char_t * disp_start = console->disp_base + console->display_cols * start;
    volatile disp_char_t * disp_end = console->disp_base + console->display_cols * (end + 1);

    while (disp_start < disp_end) {
        disp_start->c = ' ';
        disp_start->foreground = console->foreground;
        disp_start->background = console->background;

        disp_start++;
    }
}

/**
 * 整体屏幕上移若干行
 */
static void scroll_up(console_t * console, int lines) {
    // 整体上移
    disp_char_t * dest = console->disp_base;
    disp_char_t * src = console->disp_base + console->display_cols * lines;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size);

    // 擦除最后一行
    erase_rows(console, console->display_rows - lines, console->display_rows - 1);

    console->cursor_row -= lines;
}

static void move_to_col0 (console_t * console) {
	console->cursor_col = 0;
}

/**
 * 换至下一行
 */
static void move_next_line (console_t * console) {
	console->cursor_row++;

	// 超出当前屏幕显示的所有行，上移一行
	if (console->cursor_row >= console->display_rows) {
		scroll_up(console, 1);
	}
}

/**
 * 将光标往前移一个字符
 */
static void move_forward (console_t * console, int n) {
	for (int i = 0; i < n; i++) {
		if (++console->cursor_col >= console->display_cols) {
			console->cursor_col = 0;
            console->cursor_row++;
            if (console->cursor_row >= console->display_rows) {
                // 超出末端，上移
                scroll_up(console, 1);
            }
        }
	}
}

/**
 * 在当前位置显示一个字符
 */
static void show_char(console_t * console, char c) {
    // 每显示一个字符，都进行计算，效率有点低。不过这样直观简单
    int offset = console->cursor_col + console->cursor_row * console->display_cols;

    disp_char_t * p = console->disp_base + offset;
    p->c = c;
    p->foreground = console->foreground;
    p->background = console->background;
    move_forward(console, 1);
}

/**
 * 光标左移
 * 如果左移成功，返回0；否则返回-1
 */
static int move_backword (console_t * console, int n) {
    int status = -1;

    for (int i = 0; i < n; i++) {
        if (console->cursor_col > 0) {
            // 非列超始处,可回退
            console->cursor_col--;
            status = 0;
        } else if (console->cursor_row > 0) {
            // 列起始处，但非首行，可回腿
            console->cursor_row--;
            console->cursor_col = console->display_cols - 1;
            status = 0;
        }
    }

    return status;
}

static void clear_display (console_t * console) {
    int size = console->display_cols * console->display_rows;

    disp_char_t * start = console->disp_base;
    for (int i = 0; i < size; i++, start++) {
        // 为便于理解，以下分开三步写一个字符，速度慢一些
        start->c = ' ';
        start->background = console->background;
        start->foreground = console->foreground;
    }
}

/**
 * 将光标对齐到8的倍数位置上
 */
static void move_next_tab(console_t * console) {
    int col = console->cursor_col;

    col = (col + 7) / 8 * 8;		// 下一显示位置
    if (col >= console->display_cols) {
        col = 0;
        console->cursor_row++;
        if (console->cursor_row >= console->display_rows) {
            // 超出末端，上移
            scroll_up(console, 1);
        }
    }
    console->cursor_col = col;
}

/**
 * 只支持保存光标
 */
void save_cursor(console_t * console) {
    console->old_cursor_col = console->cursor_col;
    console->old_cursor_row = console->cursor_row;
}

void restore_cursor(console_t * console) {
    console->cursor_col = console->old_cursor_col;
    console->cursor_row = console->old_cursor_row;
}

/**
 * 初始化控制台及键盘
 */
int console_init (int idx) {
    console_t *console = console_buf + idx;

    console->display_cols = CONSOLE_COL_MAX;
    console->display_rows = CONSOLE_ROW_MAX;
    console->disp_base = (disp_char_t *) CONSOLE_DISP_ADDR + idx * console->display_cols * console->display_rows;

    console->foreground = COLOR_White;
    console->background = COLOR_Black;
    if (idx == 0) {
        int cursor_pos = read_cursor_pos();
        console->cursor_row = cursor_pos / console->display_cols;
        console->cursor_col = cursor_pos % console->display_cols;
    } else {
        console->cursor_row = 0;
        console->cursor_col = 0;
        clear_display(console);
    }

    console->old_cursor_row = console->cursor_row;
    console->old_cursor_col = console->cursor_col;

    mutex_init(&console->mutex);
	return 0;
}


/**
 * 擦除前一字符
 * @param console
 */
static void erase_backword (console_t * console) {
    if (move_backword(console, 1) == 0) {
        show_char(console, ' ');
        move_backword(console, 1);
    }
}

/**
 * 普通状态下的字符的写入处理
 */
static void write_normal (console_t * console, char c) {
    switch (c) {
        case ASCII_ESC:
            console->write_state = CONSOLE_WRITE_ESC;
            break;
        case 0x7F:
            erase_backword(console);
            break;
        case '\b':		// 左移一个字符
            move_backword(console, 1);
            break;
            // 换行处理
        case '\t':		// 对齐的下一制表符
            move_next_tab(console);
            break;
        case '\r':
            move_to_col0(console);
            break;
        case '\n':  // 暂时这样处理
            //move_to_col0(console);
            move_next_line(console);
            break;
            // 普通字符显示
        default: {
            if ((c >= ' ') && (c <= '~')) {
                show_char(console, c);
            }
            break;
        }
    }
}

/**
 * 清空参数表
 */
static void clear_esc_param (console_t * console) {
	kernel_memset(console->esc_param, 0, sizeof(console->esc_param));
	console->curr_param_index = 0;
}

/**
 * 写入以ESC开头的序列
 */
static void write_esc (console_t * console, char c) {
    // https://blog.csdn.net/ScilogyHunter/article/details/106874395
    // ESC状态处理, 转义序列模式 ESC 0x20-0x27(0或多个) 0x30-0x7e
    switch (c) {
        case '7':		// ESC 7 保存光标
            save_cursor(console);
            console->write_state = CONSOLE_WRITE_NORMAL;
            break;
        case '8':		// ESC 8 恢复光标
            restore_cursor(console);
            console->write_state = CONSOLE_WRITE_NORMAL;
            break;
        case '[':
            clear_esc_param(console);
            console->write_state = CONSOLE_WRITE_SQUARE;
            break;
        default:
            console->write_state = CONSOLE_WRITE_NORMAL;
            break;
    }
}

/**
 * 设置字符属性
 */
static void set_font_style (console_t * console) {
	static const cclor_t color_table[] = {
			COLOR_Black, COLOR_Red, COLOR_Green, COLOR_Yellow, // 0-3
			COLOR_Blue, COLOR_Magenta, COLOR_Cyan, COLOR_White, // 4-7
	};

	for (int i = 0; i < console->curr_param_index; i++) {
		int param = console->esc_param[i];
		if ((param >= 30) && (param <= 37)) {  // 前景色：30-37
			console->foreground = color_table[param - 30];
		} else if ((param >= 40) && (param <= 47)) {
			console->background = color_table[param - 40];
		} else if (param == 39) { // 39=默认前景色
			console->foreground = COLOR_White;
		} else if (param == 49) { // 49=默认背景色
			console->background = COLOR_Black;
		}
	}
}

/**
 * @brief 光标左移，但不起始左边界，也不往上移
 */
static void move_left (console_t * console, int n) {
    // 至少移致动1个
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col - n;
    console->cursor_col = (col >= 0) ? col : 0;
}

/**
 * @brief 光标右移，但不起始右边界，也不往下移
 */
static void move_right (console_t * console, int n) {
    // 至少移致动1个
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col + n;
    if (col >= console->display_cols) {
        console->cursor_col = console->display_cols - 1;
    } else {
        console->cursor_col = col;
    }
}

/**
 * 移动光标
 */
static void move_cursor(console_t * console) {
	if (console->curr_param_index >= 1) {
		console->cursor_row = console->esc_param[0];
	}

	if (console->curr_param_index >= 2) {
		console->cursor_col = console->esc_param[1];
	}
}

/**
 * 擦除字符操作
 */
static void erase_in_display(console_t * console) {
	if (console->curr_param_index <= 0) {
		return;
	}

	int param = console->esc_param[0];
	if (param == 2) {
		// 擦除整个屏幕
		erase_rows(console, 0, console->display_rows - 1);
        console->cursor_col = console->cursor_row = 0;
	}
}

/**
 * @brief 处理ESC [Pn;Pn 开头的字符串
 */
static void write_esc_square (console_t * console, char c) {
    // 接收参数
    if ((c >= '0') && (c <= '9')) {
        // 解析当前参数
        int * param = &console->esc_param[console->curr_param_index];
        *param = *param * 10 + c - '0';
    } else if ((c == ';') && console->curr_param_index < ESC_PARAM_MAX) {
        // 参数结束，继续处理下一个参数
        console->curr_param_index++;
    } else {
        // 结束上一字符的处理
        console->curr_param_index++;

        // 已经接收到所有的字符，继续处理
        switch (c) {
        case 'm': // 设置字符属性
            set_font_style(console);
            break;
        case 'D':	// 光标左移n个位置 ESC [Pn D
            move_left(console, console->esc_param[0]);
            break;
        case 'C':
            move_right(console, console->esc_param[0]);
            break;
        case 'H':
        case 'f':
            move_cursor(console);
            break;
        case 'J':
            erase_in_display(console);
            break;
        }
        console->write_state = CONSOLE_WRITE_NORMAL;
    }
}

/**
 * 实现pwdget作为tty的输出
 * 可能有多个进程在写，注意保护
 */
int console_write (tty_t * tty) {
	console_t * console = console_buf + tty->console_idx;

    // 下面的写序列涉及到状态机，还有多进程同时写，因此加上锁
    mutex_lock(&console->mutex);

    int len = 0;
    do {
        char c;

        // 取字节数据
        int err = tty_fifo_get(&tty->ofifo, &c);
        if (err < 0) {
            break;
        }
        sem_notify(&tty->osem);

        // 显示出来
        switch (console->write_state) {
            case CONSOLE_WRITE_NORMAL: {
                write_normal(console, c);
                break;
            }
            case CONSOLE_WRITE_ESC:
                write_esc(console, c);
                break;
            case CONSOLE_WRITE_SQUARE:
                write_esc_square(console, c);
                break;
        }
        len++;
    }while (1);

    mutex_unlock(&console->mutex);

    update_cursor_pos(console);
    return len;
}

/**
 * @brief 关闭控制台及键盘
 */
void console_close (int dev) {
	// 似乎不太需要做点什么
}
