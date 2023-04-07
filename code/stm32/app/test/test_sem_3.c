
#if 0

#include "os_api.h"

int task1_flag;
int task2_flag;
int task3_flag;
int task4_flag;

#define BUFFER_CNT		100
static int buffer[BUFFER_CNT];
static os_sem_t read_sem;      // 读信号量
static os_sem_t write_sem;     // 写信号量
static int write_idx, read_idx, total;  // 写索引和读索引，总计数

void task1_entry (void * arg) {
	int idx = 0;
	task1_flag = 0;
	
	for (;;) {
		// 等待有数据，即读信号量中的数据资源计数不为0
		os_sem_wait(&read_sem, 0);

		// 取出一个数据
		int data = buffer[read_idx++];
		if (read_idx >= BUFFER_CNT) {
			read_idx = 0;
		} 

		// 显示当前读的内容
		printf("thread 1 read data=%d, data_cnt=%d, free cnt=%d, size=%d\n", 
					data, os_sem_cnt(&read_sem), os_sem_cnt(&write_sem), os_sem_max(&write_sem));

		// 发信号给写线程，通知可以进行写了
		os_sem_release(&write_sem);

		// 每30个快速读、每30个快速写
		if (((idx++ / 30) & 0x1) == 0) {
			os_task_sleep(100);
		}
	}
}

void task2_entry (void * arg) {
	int data = 0;
	task2_flag = 0;
	
	for (;;) {
		// 写之前要等有可用的写空间，看看写信号量中空闲资源的数量不为0
		os_sem_wait(&write_sem, 0);

		// 不为0则将数据写入
		buffer[write_idx++] = data++;
		if (write_idx >= BUFFER_CNT) {
			write_idx = 0;
		} 
		// 显示当前写的内容，方便观察
		printf("thread 2 write data=%d\n", data);

		// 通知另一个线程可以进行读取，但具体什么时候读则不确定
		os_sem_release(&read_sem);
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

int first_flags;
void os_app_init (void) {
	first_flags = 0;

	os_sem_init(&read_sem, 0, 0);
	os_sem_init(&write_sem, BUFFER_CNT, BUFFER_CNT);

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
	first_flags ^= 1;	
	os_task_sleep(100);
}

#endif
