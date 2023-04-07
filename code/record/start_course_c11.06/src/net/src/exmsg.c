#include "exmsg.h"
#include "sys_plat.h"
#include "fixq.h"
#include "dbg.h"
#include "mblock.h"

static void * msg_tbl[EXMSG_MSG_CNT];
static fixq_t msg_queue;

static exmsg_t msg_buffer[EXMSG_MSG_CNT];
static mblock_t msg_block;

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

        pktbuf_fill(buf, 0x11, 6);

        net_err_t err = netif_out(netif, (ipaddr_t *)0, buf);
        if (err < 0) {
            pktbuf_free(buf);
        }

        
    }

    return NET_ERR_OK;
}

static void work_thread (void * arg) {
    dbg_info(DBG_MSG, "exmsg is running....\n");

    while (1) {
        exmsg_t * msg = (exmsg_t *)fixq_recv(&msg_queue, 0);

        dbg_info(DBG_MSG, "recv a msg %p: %d\n", msg, msg->type);
        switch (msg->type)
        {
        case NET_EXMSG_NETIF_IN:
            do_netif_in(msg);
            break;
        
        default:
            break;
        }

        mblock_free(&msg_block, msg);
    }
}

net_err_t exmsg_start (void) {
    sys_thread_t thread = sys_thread_create(work_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }
    return NET_ERR_OK;
}
