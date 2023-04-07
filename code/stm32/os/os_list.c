#include "os_list.h"
#include "os_task.h"
#include "os_cfg.h"

const char * os_err_str (os_err_t err) {
    return "1234";
}

/**
 * 链表完整性检查函数，用于检查整个任务链表是否正确
 * 当发现不正确时，程序死机，卡死在指定的位置
 */
#if OS_DEBUG_ENABLE && OS_ENABLE_DEBUG_LIST
static void os_list_check (os_list_t * list) {
    os_assert(list != OS_NULL);

    os_task_t * first = list->first;
    os_task_t * last = list->last;

    // 检查list中的first+last+cnt
    os_assert(list->cnt >= 0);
    if (list->cnt == 0) {
        // 为空时，first和last必定为0
        os_assert(!list->first && !list->last);
    } else {
        // 不为空时，二者必定不为0
        os_assert(list->first && list->last);
        if (list->cnt == 1) {
            os_assert(list->first == list->last);         // 1个，first和last必须相同
        } else {
            os_assert(list->first != list->last);          // 超过1个，不能指向同一个
        }
    }
 
    // 检查链接关系
    int cnt = 0;
    for (os_task_t * curr = list->first; curr; curr = curr->next) {
        // 不是第一个，pre肯定不为空；否则肯定为空
        if (curr == first) {
            os_assert(curr->pre == OS_NULL);
        } else {
            os_assert(curr->pre != OS_NULL);
        }

        // 不是最后一个，next肯定不为空；否则肯定为空
        if (curr == last) {
            os_assert(curr->next == OS_NULL);
        } else {
            os_assert(curr->next != OS_NULL);
        }

        // 检查前后结点的指向
        if (curr->next) {
            os_assert(curr->next->pre == curr);
        } 
        if (curr->pre) {
            os_assert(curr->pre->next == curr);
        }

        // TODO: 可以在此做一些任务结构方面的检查
        cnt++;
    }    

    // 计数要相同
    os_assert(cnt == list->cnt);
}

/**
 * 以列表的形式显示整个任务列表中的情况
 */
void os_list_show (os_list_t * list,  const char * title) {
    os_assert(list != OS_NULL);
    os_assert(title != OS_NULL);

    if (list->first == OS_NULL) {
        printf("%s: list emptry\n", title);
        return;
    }

    printf("%s\n", title);

    int idx = 0, total_ms = 0;
    os_task_t * task = os_list_first(list);
    while (task) {
        total_ms += task->delay_tick * OS_SYSTICK_MS;
        printf("%d: %s %d %d\n", idx++, task->name, task->delay_tick  * OS_SYSTICK_MS, total_ms);

        task = task->next;
    }
}
#else
#define os_list_check(list)
#define os_list_show(list, title)
#endif 

/**
 * 初始化任务队列
 */
void os_list_init (os_list_t * list) {
    os_assert(list != OS_NULL);

	list->first = list->last = OS_NULL;
    list->cnt = 0;
}

/**
 * 返回任务列表中结点的数量
 * @param list 任务列表
 * @return 任务数量
 */
int os_list_cnt (os_list_t * list) {
    os_assert(list != OS_NULL);

	return list->cnt;
}

/**
 * 返回任务列表的第一个任务
 * @param list 任务列表
 * @return 第一个任务
 */
os_task_t * os_list_first (os_list_t * list) {
    os_assert(list != OS_NULL);

	if (list->cnt) {
		return list->first;
	}    
    return  OS_NULL;
}

/**
 * 返回任务列表的最后一个任务
 * @param list 任务列表
 * @return 最后一个任务
 */
os_task_t * os_list_last (os_list_t * list) {
    os_assert(list != OS_NULL);

	if (list->cnt) {
		return list->last;
	}    
    return  OS_NULL;
}

/**
 * 获取指定任务列表的后一任务
*/
struct _os_task_t * os_list_next (os_list_t * list, struct _os_task_t * task) {
    os_assert(list != OS_NULL);
    os_assert(task != OS_NULL);
    return task->next;
}

/**
 * 清空任务列表，即将其中的任务全部从队列中移除
 * @param list 任务列表
 */
void os_list_clear (os_list_t * list) {
    os_assert(list != OS_NULL);

    os_task_t * curr = list->first;

    // 遍历链表，逐个移除所有结点
    while (curr) {
        os_task_t * next = curr->next;

        // 重置结点
        curr->pre = curr->next = OS_NULL;
        curr = next;
    }

    // 重置链表结构
    list->first = list->last = OS_NULL;
    list->cnt = 0;

    os_list_check(list);
}

/**
 * 将指定任务添加到队列头部
 * @param list 任务列表
 * @param task 待加入的任务
 */
void os_list_add_head (os_list_t * list, os_task_t * task) {
    os_assert(list != OS_NULL);

    // 结点本身
    task->pre = OS_NULL;
    task->next = list->first;
    
    // 如果原来有首结点，则更改其pre；否则作为新的首结点
    if (list->first) {
        list->first->pre = task;
		list->first = task;
    } else {
        // 原链表为空
        list->first = task;
        list->last = task;
    }

    list->cnt++;
    os_list_check(list);
}

/**
 * 将指定任务添加到队列尾部
 * @param list 任务列表
 * @param task 待加入的任务
 */
void os_list_append (os_list_t * list, os_task_t * task) {
    os_assert(list != OS_NULL);
    os_assert(task != OS_NULL);

    // 结点本身
    task->next = OS_NULL;
    task->pre = list->last;

    // 如果原来有尾结点，则更改其next；否则作为新的尾结点
    if (list->last) {
        list->last->next = task;
		list->last = task;
    } else {
        // 原链表为空
        list->first = list->last = task;
    }

    list->cnt++;
    os_list_check(list);
}

/**
 * 移除列表头部的任务
 * @param list 任务列表
 * @return 移除的任务
 */
os_task_t * os_list_remove_head (os_list_t * list) {
    os_assert(list != OS_NULL);

    os_task_t * task = OS_NULL;
    if (list->cnt > 0) {
        task = list->first;

        list->first = task->next;
        if (list->first) {
            list->first->pre = OS_NULL;        // 新头结点的pre
        }

        // 只有一个结点，last=null,first前置已经置为0
        if (list->last == task) {
            list->last = OS_NULL;
        }

        list->cnt--;
    }

    os_list_check(list);
    return task;
}

/**
 * 移除指定的任务
 * @param list 任务列表
 * @param 待移除的任务
 */
void os_list_remove (os_list_t * list, os_task_t * task) {
    os_assert(list != OS_NULL);
    os_assert(task != OS_NULL);

    if (task->pre) {
        task->pre->next = task->next;
    }

    if (task->next) {
        task->next->pre = task->pre;
    }

    if (list->first == task) {
        list->first = task->next;
    }

    if (list->last == task) {
        list->last = task->pre;
    }

    task->pre = task->next = OS_NULL;
    list->cnt--;

    os_list_check(list);
}

/**
 * 插入一个任务到指定的结点后
 * @param list 待插入的结点
 * @param pre 待插入的结点后
 * @param next 等待被插入的结点
*/
void os_list_insert_after (os_list_t * list, os_task_t * pre, os_task_t * next) {
    os_assert(list != OS_NULL);
    os_assert(next != OS_NULL);

    // 插入表头
    if (!pre) {
        os_list_add_head(list, next);
        return;
    }

    // 结点本身
    next->pre = pre;
    next->next = pre->next;

    // 前后结点，后结点可能存在也可能不存在
    pre->next = next;
    if (next->next) {
        next->next->pre = next;
    }

    // 新的尾结点
    if (list->last == pre) {
        list->last = next;
    }

    list->cnt++;

    os_list_check(list);
}
