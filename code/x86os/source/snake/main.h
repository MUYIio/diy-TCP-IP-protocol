/**
 * 贪吃蛇游戏
 *
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CMD_H
#define CMD_H

#define ESC_CMD2(Pn, cmd)		    "\x1b["#Pn#cmd
#define ESC_CLEAR_SCREEN		    ESC_CMD2(2, J)	// 擦除整屏幕

#define PLAYER1_KEY_UP			'w'
#define PLAYER1_KEY_DOWN		's'
#define PLAYER1_KEY_LEFT		'a'
#define PLAYER1_KEY_RIGHT		'd'
#define PLAYER1_KEY_QUITE		'q'

/**
 * 蛇身的一个节点
 */
typedef struct _body_part_t {
	int row;
	int col;
	struct _body_part_t *next;
}body_part_t;

/*
 * 蛇结构
 */
typedef struct _snake_t {
	body_part_t * head;

	enum {
		SNAKE_BIT_NONE,
		SNAKE_BIT_ITSELF,
		SNAKE_BIT_WALL,
		SNAKE_BIT_FOOD,
	} status;

	int dir;
}snake_t;

#endif
