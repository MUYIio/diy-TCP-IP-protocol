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

os_queue_t * task1_queue;

void queue_show (os_queue_t * queue) {
	os_dbg("queue: msg=%d, free=%d, wait=%d, release=%d\n", os_queue_msg_cnt(queue), os_queue_free_cnt(queue), 
			os_queue_wait_tasks(queue), os_queue_release_tasks(queue));
}

#define MSG_CNT		20

void task1_entry (void * arg) {
	task1_flag = 0;

	int msg;
	os_err_t err = os_queue_wait(task1_queue, 0, 0, &msg);
	if(err < 0) {
		os_dbg("queue deleted\n");
	}

	for (;;) {
		os_task_sleep(10);
		task1_flag ^= 1;
	}
}

void task2_entry (void * arg) {
	task2_flag = 0;
	
	os_task_sleep(100);
	os_queue_free(task1_queue);

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
os_queue_t first_queue;
int buf[MSG_CNT];

void os_app_init (void) {
	first_flag = 0;

	os_queue_init(&first_queue, buf, sizeof(int), MSG_CNT);
	task1_queue = os_queue_create(sizeof(int), MSG_CNT);
	
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
	int msg;
	os_queue_wait(&first_queue, 60, 0, &msg);
	os_task_sleep(40);
	first_flag ^= 1;	
}

#endif
