#include "exmsg.h"
#include "sys_plat.h"
#include "fixq.h"
#include "dbg.h"
#include "mblock.h"
#include "sys.h"
#include "timer.h"
#include "ipv4.h"

static void * msg_tbl[EXMSG_MSG_CNT];
static fixq_t msg_queue;

static exmsg_t msg_buffer[EXMSG_MSG_CNT];
static mblock_t msg_block;

net_err_t test_func (struct _func_msg_t * msg) {
    printf("hello, 1234: 0x%x", *(int *)msg->param);
    return NET_ERR_OK;
}

net_err_t exmsg_func_exec (exmsg_func_t func, void * param) {
    func_msg_t func_msg;
    func_msg.func = func;
    func_msg.param = param;
    func_msg.err = NET_ERR_OK;
    func_msg.thread = sys_thread_self();
    func_msg.wait_sem = sys_sem_create(0);
    if (func_msg.wait_sem == SYS_SEM_INVALID) {
        dbg_error(DBG_MSG, "error create wait sem");
        return NET_ERR_MEM;
    }

    exmsg_t * msg = mblock_alloc(&msg_block, 0);
    if (!msg) {
        dbg_warning(DBG_MSG, "no free msg");
        sys_sem_free(func_msg.wait_sem);
        return NET_ERR_MEM;
    }

    msg->type = NET_EXMSG_FUN;
    msg->func = &func_msg;

    dbg_info(DBG_MSG, "1. begin call fun: %p", func);
    net_err_t err = fixq_send(&msg_queue, msg, 0);
    if (err < 0) {
        dbg_warning(DBG_MSG, "fixq full");
        sys_sem_free(func_msg.wait_sem);
        mblock_free(&msg_block, msg);
        return err;
    }

    sys_sem_wait(func_msg.wait_sem, 0);
    dbg_info(DBG_MSG, "end call func: %p", func);
    return func_msg.err;
}


static net_err_t do_func (func_msg_t * func_msg) {
    dbg_info(DBG_MSG, "call func");

    func_msg->err = func_msg->func(func_msg);

    sys_sem_notify(func_msg->wait_sem);
    dbg_info(DBG_MSG, "func exec complete");
    return NET_ERR_OK;
}

net_err_t exmsg_init (void) {
    dbg_info(DBG_MSG, "exmsg init");

    net_err_t err = fixq_init(&msg_queue, msg_tbl, EXMSG_MSG_CNT, EXMSG_LOCKER);
    if (err < 0) {
        dbg_error(DBG_MSG, "fixq init failed");
        return err;
    }

    err = mblock_init(&msg_block, msg_buffer, sizeof(exmsg_t), EXMSG_MSG_CNT, EXMSG_LOCKER);
    if (err < 0) {
        dbg_error(DBG_MSG, "mblock init failed.");
        return err;
    }

    dbg_info(DBG_MSG, "init done");
    return NET_ERR_OK;
}

net_err_t exmsg_netif_in(netif_t * netif) {
    exmsg_t * msg = mblock_alloc(&msg_block, -1);
    if (!msg) {
        dbg_warning(DBG_MSG, "no free msg");
        return NET_ERR_MEM;
    }

    msg->type = NET_EXMSG_NETIF_IN;
    msg->netif.netif = netif;

    net_err_t err = fixq_send(&msg_queue, msg, -1);
    if (err < 0) {
        dbg_warning(DBG_MSG, "fixq full");
        mblock_free(&msg_block, msg);
        return err;
    }

    return err;
}


static net_err_t do_netif_in (exmsg_t * msg) {
    netif_t * netif = msg->netif.netif;

    pktbuf_t * buf;
    while ((buf = netif_get_in(netif, -1))) {
        dbg_info(DBG_MSG, "recv a packet");

        if (netif->link_layer) {
            net_err_t err = netif->link_layer->in(netif, buf);
            if (err < 0) {
                pktbuf_free(buf);
                dbg_warning(DBG_MSG, "netif in failed, error=%d", err);
            }
        } else {
            net_err_t err = ipv4_in(netif, buf);
            if (err < 0) {
                pktbuf_free(buf);
                dbg_warning(DBG_MSG, "netif in failed, error=%d", err);
            }
        }
    }

    return NET_ERR_OK;
}

static void work_thread (void * arg) {
    dbg_info(DBG_MSG, "exmsg is running....\n");

    net_time_t time;
    sys_time_curr(&time);

    while (1) {
        int first_tmo = net_timer_first_tmo();
        exmsg_t * msg = (exmsg_t *)fixq_recv(&msg_queue, first_tmo);
        if (msg) {
            dbg_info(DBG_MSG, "recv a msg %p: %d\n", msg, msg->type);
            switch (msg->type)
            {
            case NET_EXMSG_NETIF_IN:
                do_netif_in(msg);
                break;
            case NET_EXMSG_FUN:
                do_func(msg->func);
                break;
            default:
                break;
            }

            mblock_free(&msg_block, msg);
        }

        int diff_ms = sys_time_goes(&time);
        net_timer_check_tmo(diff_ms);
    }
}

net_err_t exmsg_start (void) {
    sys_thread_t thread = sys_thread_create(work_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }
    return NET_ERR_OK;
}
