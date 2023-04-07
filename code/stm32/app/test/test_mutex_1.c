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

#define LOOP_CNT		50000
#define TEST_CNT		10
os_mutex_t * mutex;
int count = 0;

void task1_entry (void * arg) {
	task1_flag = 0;
	
	for (int i = 0; i < TEST_CNT; i++) {
		os_mutex_lock(mutex, 0);
		for (int i = 0; i < LOOP_CNT; i++) {
			count++;
		}

		printf("mutex owner: %s, lock cnt: %d, wait task: %d, prio: %d\n", 
				os_mutex_owner(mutex)->name, os_mutex_lock_cnt(mutex), 
				os_mutex_tasks(mutex), os_task_self()->prio);

		os_mutex_unlock(mutex);
		printf("task1: count=%d\n", count);

		os_task_sleep(10);
	}

	for (;;) {
		os_task_sleep(10);
		task1_flag ^= 1;
	}
}

void task2_entry (void * arg) {
	task2_flag = 0;
	
	for (int i = 0; i < TEST_CNT; i++) {
		os_mutex_lock(mutex, 0);
		for (int i = 0; i < LOOP_CNT; i++) {
			count--;
		}
		printf("mutex owner: %s, lock cnt: %d, wait task: %d, prio: %d\n", 
				os_mutex_owner(mutex)->name, os_mutex_lock_cnt(mutex), 
				os_mutex_tasks(mutex), os_task_self()->prio);
		os_mutex_unlock(mutex);
		printf("task2: count=%d\n", count);

		os_task_sleep(10);
	}
	
	os_task_sleep(100);
	os_mutex_free(mutex);
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

os_mutex_t first_mutex;

void task4_entry (void * arg) {
	task4_flag = 0;
	
	for (;;) {
		os_mutex_lock(&first_mutex, 40);
		//os_task_sleep(40);
		task4_flag ^= 1;
	}
}

int first_flag;
void os_app_init (void) {
	first_flag = 0;

	os_mutex_init(&first_mutex);
	os_mutex_lock(&first_mutex, 0);

	mutex = os_mutex_create();

	os_task_t * task1 = os_task_create("task1", task1_entry, OS_NULL, 0, 1024);
	os_task_start(task1);
	os_task_t * task2 = os_task_create("task2", task2_entry, OS_NULL, 1, 1024);
	os_task_start(task2);
	os_task_t * task3 = os_task_create("task3", task3_entry, OS_NULL, 2, 1024);
	os_task_start(task3);
	os_task_t * task4 = os_task_create("task4", task4_entry, OS_NULL, 2, 1024);
	os_task_start(task4);
}

void os_app_loop(void) {
	os_task_sleep(40);
	first_flag ^= 1;	
}

#endif
