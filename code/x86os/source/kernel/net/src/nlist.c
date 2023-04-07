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

/**
 * 移除指定链表的中的表项
 * 不检查node是否在结点中
 */
nlist_node_t * nlist_remove(nlist_t *list, nlist_node_t *remove_node) {
    // 如果是头，头往前移
    if (remove_node == list->first) {
        list->first = remove_node->next;
    }

    // 如果是尾，则尾往回移
    if (remove_node == list->last) {
        list->last = remove_node->pre;
    }

    // 如果有前，则调整前的后继
    if (remove_node->pre) {
        remove_node->pre->next = remove_node->next;
    }

    // 如果有后，则调整后往前的
    if (remove_node->next) {
        remove_node->next->pre = remove_node->pre;
    }

    // 清空node指向
    remove_node->pre = remove_node->next = (nlist_node_t*)0;
    --list->count;
    return remove_node;
}

/**
 * 将指定表项插入到指定链表的尾部
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void nlist_insert_last(nlist_t *list, nlist_node_t *node) {
    // 设置好结点本身
    node->pre = list->last;
    node->next = (nlist_node_t*)0;

    // 表空，则first/last都指向唯一的node
    if (nlist_is_empty(list)) {
        list->first = list->last = node;
    } else {
        // 否则，调整last结点的向一指向为node
        list->last->next = node;

        // node变成了新的后继结点
        list->last = node;
    }

    list->count++;
}

/**
 * 将Node插入指定结点之后
 * @param 操作的链表
 * @param pre 前一结点
 * @param node 待插入的结点
 */
void nlist_insert_after(nlist_t* list, nlist_node_t* pre, nlist_node_t* node) {
    // 原链表为空
    if (nlist_is_empty(list)) {
        nlist_insert_first(list, node);
        return;
    }

    // node的下一结点，应当为pre的下一结点
    node->next = pre->next;
    node->pre = pre;

    // 先调整后粥，再更新自己
    if (pre->next) {
        pre->next->pre = node;
    }
    pre->next = node;


    // 如果pre恰好位于表尾，则新的表尾就需要更新成node
    if (list->last == pre) {
        list->last = node;
    }

    list->count++;
}

