#ifndef OS_EFLAGS_H
#define OS_EFLAGS_H

#include "os_cfg.h"
#include "os_event.h"

typedef uint32_t os_flags_t;

/**
 * 事件标志结构
*/
typedef struct _os_eflags_t {
    os_event_t event;           // 写事件结构
    os_flags_t flags;            // 当前的事件标志
}os_eflags_t;

#define	OS_EOPT_CLEAR		    (0x0 << 0)
#define	OS_EOPT_SET			    (0x1 << 0)
#define	OS_EOPT_ANY			    (0x0 << 1)
#define	OS_EOPT_ALL			    (0x1 << 1)
#define	OS_EOPT_EXIT_CLEAR       (0x1 << 7)      // 退出时清除相关标志

#define OS_EOPT_SET_ALL		    (OS_EOPT_SET | OS_EOPT_ALL)
#define	OS_EOPT_SET_ANY		    (OS_EOPT_SET | OS_EOPT_ANY)
#define OS_EOPT_CLEAR_ALL	    (OS_EOPT_CLEAR | OS_EOPT_ALL)
#define OS_EOPT_CLEAR_ANY	    (OS_EOPT_CLEAR | OS_EOPT_ANY)

os_err_t os_eflags_init (os_eflags_t * eflags, os_flags_t init_flags);
os_err_t os_eflags_uninit(os_eflags_t * eflags);
os_eflags_t * os_eflags_create (os_flags_t init_flags);
os_err_t os_eflags_free (os_eflags_t * eflags);
os_flags_t os_eflags_wait (os_eflags_t * eflags, int ms, int type, os_flags_t flags,  os_err_t * p_err);
os_flags_t os_eflags_flags (os_eflags_t * eflags, os_err_t * err);
os_err_t os_eflags_release (os_eflags_t * eflags, int type, os_flags_t flags);
int os_eflags_tasks (os_eflags_t * eflags);

#endif 

