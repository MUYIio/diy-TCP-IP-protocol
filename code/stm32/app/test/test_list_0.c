#if 0

#include "os_list.h"
#include "os_task.h"

#define TASK_CNT        4

static os_task_t task_table[TASK_CNT] = {
    [0] = {.name = "task0"},
    [1] = {.name = "task1"},
    [2] = {.name = "task2"},
    [3] = {.name = "task3"},
};

static os_list_t task_list;

void os_app_init (void) {
    os_list_init(&task_list);

    // 头部添加和移除
    printf("\nadd head:\n");
    for (int i = 0; i < TASK_CNT; i++) {
        os_task_t * task = task_table + i;
        os_list_add_head(&task_list, task);
        printf("%s\n", task->name);
    }
    os_list_show(&task_list, "before remove");

    for (int i = 0; i < TASK_CNT; i++) {
        os_task_t * task = os_list_remove_head(&task_list);
        printf("%s\n", task->name);
        os_assert(task == task_table + TASK_CNT - 1 - i);
    }    
    os_list_show(&task_list, "after remove");

    // 尾部添加和移除
    printf("\nadd tail:\n");
    for (int i = 0; i < TASK_CNT; i++) {
        os_task_t * task = task_table + i;
        os_list_append(&task_list, task);
        printf("%s\n", task->name);
    }
    os_list_show(&task_list, "before remove");

    for (int i = 0; i < TASK_CNT; i++) {
        os_task_t * task = os_list_remove_head(&task_list);
        printf("%s\n", task->name);
        os_assert(task == task_table + i);
    }    
    os_list_show(&task_list, "after remove");

    // 中间添加移除
    printf("\nadd after:\n");
    for (int i = 0; i < TASK_CNT; i++) {
        os_task_t * task = task_table + i;
        os_list_insert_after(&task_list, os_list_first(&task_list), task);        
        printf("%s\n", task->name);
    }
    os_list_show(&task_list, "before remove");

    printf("\nclear:\n");
    for (int i = 0; i < TASK_CNT; i++) {
        os_task_t * task = task_table + i;
        os_list_remove(&task_list, task);
        printf("%s\n", task->name);
    }
    os_list_show(&task_list, "after remove");

    // 清空链表
    for (int i = 0; i < TASK_CNT; i++) {
        os_list_append(&task_list, task_table + i);
    }
    os_list_show(&task_list, "before remove");

    os_list_clear(&task_list);    
    os_list_show(&task_list, "after remove");
}

void os_app_loop(void) {
}

#endif
