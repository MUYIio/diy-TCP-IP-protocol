/**
 * @file clist.h
 * @author lishutong (527676163@qq.com)
 * @brief 通用链表结构
 * 整个网络协议栈使用最多的基本数据结构。
 * 对于任何要使用链表的地方，只需要在相应的结构体中使用nlist_node_t
 * 然后即可将其作为链表结点即可
 * @version 0.1
 * @date 2022-10-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "nlist.h"

/**
 * 初始化链表
 * @param list 待初始化的链表
 */
void nlist_init(nlist_t *list) {
    list->first = list->last = (nlist_node_t *)0;
    list->count = 0;
}

/**
 * 将指定表项插入到指定链表的头部
 * @param list 待插入的链表
 * @param node 待插入的结点
 */
void nlist_insert_first(nlist_t *list, nlist_node_t *node) {
    // 设置好待插入结点的前后，前面为空
    node->next = list->first;
    node->pre = (nlist_node_t *)0;

    // 如果为空，需要同时设置first和last指向自己
    if (nlist_is_empty(list)) {
        list->last = list->first = node;
    } else {
        // 否则，设置好原本第一个结点的pre
        list->first->pre = node;

        // 调整first指向
        list->first = node;
    }

    list->count++;
}

