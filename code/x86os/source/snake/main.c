/**
 * 贪吃蛇游戏
 *
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "main.h"
#include "applib/lib_syscall.h"
#include <stdlib.h>
#include <stdio.h>

// 显示相关
static int row_max, col_max;

// 蛇相关
static body_part_t * food;
static snake_t snake;		// 目前只支持一条蛇

/**
 * 判断蛇是否咬到自己
 */
static int is_hit_itself (void) {
	for (body_part_t * body = snake.head->next; body; body = body->next) {
		if ((body->row == snake.head->row) && (body->col == snake.head->col)) {
			return 1;
		}
	}
	return 0;
}

/**
 * 判断是否碰到墙
 */
static int is_hit_wall (void) {
	return ((snake.head->row <= 0) 
			|| (snake.head->col <= 0))
			|| (snake.head->row >= row_max - 1) 
			|| (snake.head->col >= col_max - 1);
}

/**
 * 判断是否吃到食物
 */
static int is_hit_food (void) {
	return ((snake.head->row == food->row) && (snake.head->col == food->col));
}

static inline void show_char(int row, int col, char c) {
    printf("\x1b[%d;%dH%c\x1b[%d;%dH", row, col, c, row, col);
}

static inline void show_string (int row, int col, char * str) {
    printf("\x1b[%d;%dH%s", row, col, str);
}

/**
 * 清空整个地图
 */
void clear_map (void) {
    printf("%s", ESC_CLEAR_SCREEN);
}

/**
 * 创建地图
 */
void create_map(void) {
	clear_map();

    // 上下行
    for (int col = 1; col < col_max - 1; col++) {
        show_char(0, col, '=');
    }
    for (int col = 1; col < col_max - 1; col++) {
        show_char(row_max -1, col, '=');
    }

    // 左右边界
    for (int row = 1; row < row_max - 1; row++) {
        show_char(row, 0, '|');
    }
    for (int row = 1; row < row_max - 1; row++) {
        show_char(row, col_max - 1, '|');
    }
}

/**
 * 给蛇添加头部
 */
static void add_head (int row, int col) {
	body_part_t * part = (body_part_t *)malloc(sizeof(body_part_t));
	part->row = row;
	part->col = col;
	part->next = snake.head;
	snake.head = part;
	show_char(row, col, '*');
}

/**
 * 移除蛇尾的最后一个结点
 */
static void remove_tail (void) {
	// 先定位curr到最后一个结点
	body_part_t * pre = (body_part_t *)0;
	body_part_t * curr = snake.head;
	while (curr->next) {
		pre = curr;
		curr = curr->next;
	}

	show_char(curr->row, curr->col, ' ');

	// 再移除
	pre->next = curr->next;
	curr->next = (body_part_t *)0;
	free(curr);
}

/**
 * 创建蛇。最开始只有一个头
 */
static void create_snake (void) {
	snake.head = (body_part_t *)malloc(sizeof(body_part_t));
	snake.head->row = 10;        // 初始位置
	snake.head->col = 20;
	snake.head->next = (body_part_t *)0;
	snake.status = SNAKE_BIT_NONE;
	snake.dir = PLAYER1_KEY_LEFT;
    show_char(snake.head->row, snake.head->col, '*');
}

/**
 * 创建食物
 */
static void create_food(void)  {
	// 创建一个body
	food = (body_part_t *)malloc(sizeof(body_part_t));

	// 在设定位置时要避免与身体重合，所以要反复调整
	body_part_t * part = snake.head;
	do {
		// 设定一个随机的位置，没有随机数怎么办？
		// 只要让每次出现的位置有变化即可，不一定要真的随机
		food->row = 1 + rand() % (row_max - 3);
		food->col = 1 + rand() % (col_max - 3);

		// 食物不能在墙上
		if ((food->row < 0) || (food->row >= row_max)
				|| (food->col < 0) || (food->col >= col_max)) {
			continue;
		}

		// 食物不能在蛇身上
		// 遍历，如果有重合，则重来。没有则退出
		while (part) {
			if ((food->row != snake.head->row) || (food->col != snake.head->col)) {
				// 将食物显示出来
				show_char(food->row, food->col, '*');
				return;
			}
			part = part->next;
		}
		part = snake.head;
	} while (1);
}

/**
 * 释放掉食物
 */
static void free_food (void) {
	free(food);
	food = (body_part_t *)0;
}

/**
 * 蛇向前移动一个位置，具体移动方向由键盘控制
 */
static void move_forward (int dir) {
	int next_row, next_col;

	// 计算下一位置
	switch (dir) {
	case PLAYER1_KEY_LEFT:
		next_row = snake.head->row;
		next_col = snake.head->col - 1;
		break;
	case PLAYER1_KEY_RIGHT:
		next_row = snake.head->row;
		next_col = snake.head->col + 1;
		break;
	case PLAYER1_KEY_UP:
		next_row = snake.head->row - 1;
		next_col = snake.head->col;
		break;
	case PLAYER1_KEY_DOWN:
		next_row = snake.head->row + 1;
		next_col = snake.head->col;
		break;
	default:
		return;
	}

	body_part_t * next_part = snake.head->next;
	if (next_part) {
		if ((next_row == next_part->row) && (next_col == next_part->col)) {
			return;
		}
	}

	// 先不管有没有食物，都生成一个头部，然后前移
	add_head(next_row, next_col);

	// 然后检查相应的异常情况
	if (is_hit_itself()) {
		snake.status = SNAKE_BIT_ITSELF;
		remove_tail();
	} else if (is_hit_wall()) {
		snake.status = SNAKE_BIT_WALL;
		remove_tail();
	} else if (is_hit_food()){
		// 食物被吃掉, 回收，重新生成
		free_food();
		create_food();
		snake.status = SNAKE_BIT_FOOD;
	} else {
		// 没有吃到食物，需要移除尾部
		remove_tail();
		snake.status = SNAKE_BIT_NONE;
	}

	snake.dir = dir;
	fflush(stdout);
}

/**
 * 初始化游戏
 */
static void show_welcome (void) {
	clear_map();
	// setvbuf(stdout, NULL, _IONBF, 0); 调试用

    show_string(0, 0, "Welcome to sname game");
    show_string(1, 0, "Use a.w.s.d to move snake");
    show_string(2, 0, "Press any key to start game");
	fflush(stdout);
	
	// 按Q即退出，所以无缓存输入
	setvbuf(stdin, NULL, _IONBF, 0);
    getchar();
}

/**
 * @brief 开始游戏
 */
static void begin_game (void) {
    create_map();
    create_snake();
    create_food();
	fflush(stdout);
}

int main (int argc, char ** argv) {
	row_max = 25;
	col_max = 80;

	ioctl(0, TTY_CMD_ECHO, 0, 0);

	show_welcome();
    begin_game();

    int count;
	int cnt = 0;
	do {
		// 检查是否有键盘输入
		ioctl(0, TTY_CMD_IN_COUNT, (int)&count, 0);
		if (count) {
			int ch = getchar();
			move_forward(ch);
		} else if (++cnt % 50 == 0) {
			// 每隔一段时间自动往前移
			move_forward(snake.dir);
		}

		if ((snake.status == SNAKE_BIT_ITSELF) || (snake.status == SNAKE_BIT_WALL)) {
			int row = row_max / 2, col = col_max / 2;
			show_string(row, col,  "GAME OVER");
			show_string(row + 1, col,  "Press Any key to continue");
			fflush(stdout);
			getchar();
			break;
		}

		msleep(10);
	}while (1);

	// 这里是有危险的，如果进程异常退出，将导致回显失败
	ioctl(0, TTY_CMD_ECHO, 1, 0);
	clear_map();
    return 0;
}

