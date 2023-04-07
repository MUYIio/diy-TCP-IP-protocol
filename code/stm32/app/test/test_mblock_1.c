/**
 * 信号量的简单测试
 * 创建信号量、等待超时，并使用信号量实现任务间的同步
 */
#if 0

#include "os_api.h"

int task1_flag;
int task2_flag;
int task3_flag;
int task4_flag;

#define MEM_BLK_SIZE		16
#define MEM_BLK_CNT			4

os_mblock_t * task1_mblock;


void task1_entry (void * arg) {
	task1_flag = 0;

	void * mem[MEM_BLK_CNT];
	
	for (int i = 0; i < MEM_BLK_CNT; i++) {
		mem[i] = os_mblock_wait(task1_mblock, 0, OS_NULL);
		os_dbg("get mem block: %p\n", mem[i]);
		os_dbg("mem block cnt: %d, size: %d, wait: %d\n", os_mblock_blk_cnt(task1_mblock), 
							os_mblock_blk_size(task1_mblock), os_mblock_tasks(task1_mblock));
	}

	// 让task2运行申请内存块
	os_task_sleep(150);

	for (int i = 0; i < MEM_BLK_CNT; i++) {
		os_dbg("before reelase: mem block cnt: %d, size: %d, wait: %d\n", os_mblock_blk_cnt(task1_mblock), 
							os_mblock_blk_size(task1_mblock), os_mblock_tasks(task1_mblock));
		os_dbg("free mem block: %p\n", mem[i]);
		os_mblock_release(task1_mblock, mem[i]);

		os_task_sleep(20);
	}

	for (;;) {
		os_task_sleep(10);
		task1_flag ^= 1;
	}
}

void task2_entry (void * arg) {
	task2_flag = 0;
	
	os_task_sleep(100);
	for (int i = 0; i < MEM_BLK_CNT; i++) {
		void * msg = os_mblock_wait(task1_mblock, 0, OS_NULL);
		os_dbg("get mem block: %p\n", msg);
		os_dbg("mem block cnt: %d, size: %d, wait: %d\n", os_mblock_blk_cnt(task1_mblock), 
						os_mblock_blk_size(task1_mblock), os_mblock_tasks(task1_mblock));
	}
	
	for (;;) {
		os_task_sleep(20);
		task2_flag ^= 1;
	}
}

void task3_entry (void * arg) {
	task3_flag = 0;
	
	for (;;) {
		os_task_sleep(30);
		task3_flag ^= 1;
	}
}

void task4_entry (void * arg) {
	task4_flag = 0;
	
	for (;;) {
		os_task_sleep(40);
		task4_flag ^= 1;
	}
}

int first_flag;

void os_app_init (void) {
	first_flag = 0;

	task1_mblock = os_mblock_create(MEM_BLK_SIZE, MEM_BLK_CNT);

	os_task_t * task1 = os_task_create("task1", task1_entry, OS_NULL, 0, 1024);
	os_task_start(task1);
	os_task_t * task2 = os_task_create("task2", task2_entry, OS_NULL, 0, 1024);
	os_task_start(task2);
	os_task_t * task3 = os_task_create("task3", task3_entry, OS_NULL, 0, 1024);
	os_task_start(task3);
	os_task_t * task4 = os_task_create("task4", task4_entry, OS_NULL, 0, 1024);
	os_task_start(task4);
}

void os_app_loop(void) {	
	os_task_sleep(40);
	first_flag ^= 1;	
}

#endif
