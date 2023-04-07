#ifndef MBLOCK_H
#define MBLOCK_H

#include "nlist.h"
#include "nlocker.h"

typedef struct _mblock_t {
    nlist_t free_list;
    void * start;
    nlocker_t locker;
    sys_sem_t alloc_sem;
}mblock_t;

#endif 