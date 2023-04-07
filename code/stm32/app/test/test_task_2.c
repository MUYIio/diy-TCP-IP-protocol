#if 0

#include "os_api.h"

int first_flag = 0;

int task1_flag;
char task1_stack[1024];
os_task_t task1;

void task1_entry (void * arg) {
	task1_flag = 0;
	
	for (;;) {
		task1_flag ^= 1;
		os_task_sleep(10);
	}
}

int task2_flag;
char task2_stack[1024];
os_task_t task2;

void task2_entry (void * arg) {
	task2_flag = 0;
	
	for (;;) {
		task2_flag ^= 1;
		os_task_sleep(20);
	}
}

int task3_flag;
char task3_stack[1024];
os_task_t task3;

void task3_entry (void * arg) {
	task3_flag = 0;
	
	for (;;) {
		task3_flag ^= 1;
		os_task_sleep(30);
	}
}

void os_app_init (void) {
	os_task_init(&task1, "task1", task1_entry, OS_NULL, 0, task1_stack, sizeof(task1_stack));
	os_task_start(&task1);
	
	os_task_init(&task2, "task2", task2_entry, OS_NULL, 1, task2_stack, sizeof(task2_stack));
	os_task_start(&task2);

	os_task_init(&task3, "task3", task3_entry, OS_NULL, 1, task3_stack, sizeof(task3_stack));
	os_task_start(&task3);
}

void os_app_loop(void) {
	first_flag ^= 1;
	
	os_task_yield();
	os_task_sleep(40);
}

#endif
