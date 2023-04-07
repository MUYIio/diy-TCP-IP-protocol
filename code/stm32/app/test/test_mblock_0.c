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

void task1_entry (void * arg) {
	task1_flag = 0;
	
	for (;;) {
		os_task_sleep(10);
		task1_flag ^= 1;
	}
}

void task2_entry (void * arg) {
	task2_flag = 0;
	
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
os_mblock_t first_mblock;
static uint8_t mem_blks[4][16];

void os_app_init (void) {
	first_flag = 0;

	os_mblock_init(&first_mblock, mem_blks, 16, 4);

	uint8_t * p_mem;
	os_mblock_wait(&first_mblock, -1, (void **)&p_mem);
	os_mblock_wait(&first_mblock, -1, (void **)&p_mem);
	os_mblock_wait(&first_mblock, -1, (void **)&p_mem);
	os_mblock_wait(&first_mblock, -1, (void **)&p_mem);
	os_mblock_wait(&first_mblock, -1, (void **)&p_mem);
	os_mblock_wait(&first_mblock, -1, (void **)&p_mem);

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
	uint8_t * p_mem;
	os_mblock_wait(&first_mblock, 10, (void **)&p_mem);
	
	os_task_sleep(40);
	first_flag ^= 1;	
}

#endif
