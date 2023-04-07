/**
 * 键盘设备处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef KBD_H
#define KBD_H

#include "comm/types.h"

// https://wiki.osdev.org/%228042%22_PS/2_Controller
#define KBD_PORT_DATA			0x60
#define KBD_PORT_STAT			0x64
#define KBD_PORT_CMD			0x64

#define KBD_STAT_RECV_READY		(1 << 0)
#define KBD_STAT_SEND_FULL		(1 << 1)

// https://wiki.osdev.org/PS/2_Keyboard
#define KBD_CMD_RW_LED			0xED   // 写按键

#define KEY_RSHIFT		0x36
#define KEY_LSHIFT 		0x2A

#define KEY_CAPS			0x3A

#define KEY_E0			0xE0	// E0编码
#define KEY_E1			0xE1	// E1编码
#define	ASCII_ESC		0x1b
#define	ASCII_DEL		0x7F

/**
 * 特殊功能键
 */
#define KEY_CTRL 		0x1D		// E0, 1D或1d
#define KEY_RSHIFT		0x36
#define KEY_LSHIFT 		0x2A
#define KEY_ALT 		0x38		// E0, 38或38

#define	KEY_FUNC		 0x8000
#define KEY_F1			(0x3B)
#define KEY_F2			(0x3C)
#define KEY_F3			(0x3D)
#define KEY_F4			(0x3E)
#define KEY_F5			(0x3F)
#define KEY_F6			(0x40)
#define KEY_F7			(0x41)
#define KEY_F8			(0x42)
#define KEY_F9			(0x43)
#define KEY_F10			(0x44)
#define KEY_F11			(0x57)
#define KEY_F12			(0x58)

#define	KEY_SCROLL_LOCK		(0x46)
#define KEY_HOME			(0x47)
#define KEY_END				(0x4F)
#define KEY_PAGE_UP			(0x49)
#define KEY_PAGE_DOWN		(0x51)
#define KEY_CURSOR_UP		(0x48)
#define KEY_CURSOR_DOWN		(0x50)
#define KEY_CURSOR_RIGHT	(0x4D)
#define KEY_CURSOR_LEFT		(0x4B)
#define KEY_INSERT			(0x52)
#define KEY_DELETE			(0x53)

#define KEY_BACKSPACE		0xE

/**
 * 键盘扫描码表单元类型
 * 每个按键至多有两个功能键值
 * code1：无shift按下或numlock灯亮的值，即缺省的值
 * code2：shift按下或者number灯灭的值，即附加功能值
 */
typedef struct _key_map_t {
	uint8_t normal;				// 普通功能
	uint8_t func;				// 第二功能
}key_map_t;

/**
 * 状态指示灯
 */
typedef struct _kbd_state_t {
    int caps_lock : 1;			// 大写状态
    int lshift_press : 1;       // 左shift按下
    int rshift_press : 1;       // 右shift按下
    int ralt_press : 1;          // alt按下
    int lalt_press : 1;          // alt按下
    int lctrl_press : 1;         // ctrl键按下
    int rctrl_press : 1;         // ctrl键按下
}kbd_state_t;

void kbd_init(void);

void exception_handler_kbd (void);

#endif
