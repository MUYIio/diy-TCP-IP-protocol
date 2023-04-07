#include "ff.h"

os_sem_t * fs_mutex;
int ff_cre_syncobj (BYTE vol, _SYNC_t* sobj) {
    *sobj = os_sem_create(1, 1);
    fs_mutex = *sobj;
	return (int)(*sobj != NULL);
}

int ff_req_grant (_SYNC_t sobj) {
    os_err_t err = os_sem_wait(sobj, 0);
    return err == OS_ERR_OK;
}

void ff_rel_grant (_SYNC_t sobj) {
    os_sem_release(sobj);
}

int ff_del_syncobj (_SYNC_t sobj) {
    os_sem_free(sobj);
    return 1;
}
