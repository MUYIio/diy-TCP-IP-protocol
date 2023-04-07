#ifndef NLOCKER_H
#define NLOCKER_H

#include "sys_plat.h"

typedef enum _nlocker_type_t {
    NLOCKER_NONE,
    NLCOKER_THREAD,

}nlocker_type_t;

typedef struct _nlocker_t {
    nlocker_type_t type;

    union
    {
        sys_mutex_t mutex;
        // ...
    };
    
}nlocker_t;

#endif 