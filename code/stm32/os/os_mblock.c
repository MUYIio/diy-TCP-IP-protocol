#include "os_mblock.h"
#include "os_mem.h"

/**
 * 存储块初始化
 * @param mblock 存储块结构
 * @param mem 存储起始地址
 * @param blk_size 存储块大小
 * @param blk_cnt 存储块的数量
 * @param flags 初始化相关参数
 * @return 初始化结果
*/
static os_err_t mblock_init (os_mblock_t * mblock, void * mem, int blk_size, int blk_cnt, int flags) {
    // 初始化等待结构
    os_err_t err = os_event_init(&mblock->event, OS_EVENT_TYPE_MBLOCK, flags);
    if (err < 0) {
        os_dbg("create mblock event failed: %s.", os_err_str(err));
        return err;
    }

    mblock->mem = mem;
    mblock->blk_size = blk_size;
    mblock->blk_cnt = 0;
    mblock->list = (void *)0;

    // 构建存储块链表
	mem_link_t * start = (mem_link_t *)mem;
    for (int i = 0; i < blk_cnt; i++) {
        start->next = mblock->list;
        mblock->list = start;

        start = (mem_link_t *)((uint8_t *)start + blk_size);
        mblock->blk_cnt++;
    }   
    mblock->blk_max = mblock->blk_cnt;
    return OS_ERR_OK;
}

/**
 * 存储块初始化
 * @param mblock 存储块结构
 * @param mem 存储起始地址
 * @param blk_size 存储块大小
 * @param blk_cnt 存储块的数量
 * @return 初始化结果
 * @note 注意，传入的块大小和数量需要考虑到内存对齐等问题
*/
os_err_t os_mblock_init (os_mblock_t * mblock, void * mem, int blk_size, int blk_cnt) {
    os_param_failed(mblock == OS_NULL, OS_ERR_PARAM);
    os_param_failed(mem == OS_NULL, OS_ERR_PARAM);
    os_param_failed(blk_cnt <= 0, OS_ERR_PARAM);
    os_param_failed(blk_size < sizeof(void *), OS_ERR_PARAM);
    return mblock_init(mblock, mem, blk_size, blk_cnt, 0);
}

/**
 * 释放存储块
 */
os_err_t os_mblock_uninit (os_mblock_t * mblock) {
    os_param_failed(mblock == OS_NULL, OS_ERR_PARAM);

    // 唤醒所有任务后，尝试进行调试
    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&mblock->event);
    os_event_uninit(&mblock->event);
    os_leave_protect(protect);

    // 有任务唤醒时，进行调度，以便运行更高优先级的任务
    if (cnt > 0) {
        os_sched_run();
    } 

    return OS_ERR_OK;
}

/**
 * 分配存储块结构
 * @param blk_size 存储块大小
 * @param blk_cnt 存储块的数量
 * @return 创建的存储块
 */
os_mblock_t * os_mblock_create (int blk_size, int blk_cnt) {
    os_param_failed(blk_cnt <= 0, OS_NULL);
    os_param_failed(blk_size < sizeof(void *), OS_NULL);

    // 分配控制结构
    os_mblock_t * mblock = os_mem_alloc(sizeof(os_mblock_t));
    if (mblock == OS_NULL) {
        os_dbg("error: alloc mblock failed");
        return OS_NULL;
    }

    // 分配存储空间，注意size对齐到int大小的边界
    void * mem = os_mem_alloc(blk_size * blk_cnt);
    if (mem == OS_NULL) {
        os_dbg("error: alloc mem failed");
        os_mem_free(mblock);
        return OS_NULL;
    }

    // 初始化控制块结构
    os_err_t err = os_mblock_init(mblock, mem, blk_size, blk_cnt);
    if (err < 0) {
        os_dbg("error: init sem failed.");
        os_mem_free(mblock);
        os_mem_free(mem);
        return OS_NULL;
    }

    return mblock;
}

/**
 * 释放一个存储块
 * @param 需要释放的存储块
 */
os_err_t os_mblock_free (os_mblock_t * mblock) {
    os_param_failed(mblock == OS_NULL, OS_ERR_PARAM);

    os_mblock_uninit(mblock);
    if (mblock->event.flags & OS_EVENT_FLAG_ALLOC) {
        os_mem_free(mblock->mem);
        os_mem_free(mblock);
    }
    return OS_ERR_OK;
}

/**
 * 获取空闲的内存块数量
 * @param mblock 存储块结构
 * @return 空闲内存块数量
*/
int os_mblock_blk_cnt (os_mblock_t * mblock) {
    os_param_failed(mblock == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = mblock->blk_cnt;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取空闲的内存块大小
 * @param mblock 存储块结构
 * @return 块大小
*/
int os_mblock_blk_size (os_mblock_t * mblock) {
    os_param_failed(mblock == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = mblock->blk_size;
    os_leave_protect(protect);
    return cnt;
}

/**
 * 获取等待内存块分配的任务数量
 * @param mblock 内存控制块
 * @return 等待内存块的任务数量
 */
int os_mblock_tasks(os_mblock_t * mblock) {
    os_param_failed(mblock == OS_NULL, -1);

    os_protect_t protect = os_enter_protect();
    int cnt = os_event_wait_cnt(&mblock->event);
    os_leave_protect(protect);
    return cnt;
}

/**
 * 申请一个内存块
 * @param mblock 内存块
 * @param ms 等待时间
 * @param err 错误码
 * @return 分配的内存块
*/
void * os_mblock_wait(os_mblock_t * mblock, int ms, os_err_t * p_err) {
    os_param_failed_exec(mblock == OS_NULL, OS_NULL, if (p_err) p_err = OS_NULL);

    void * mem = OS_NULL;

    os_protect_t protect = os_enter_protect();
    if (mblock->blk_cnt > 0) {
        // 如果有内存块，则返回
        mem_link_t * mem_link = mblock->list;
        mblock->list = mem_link->next;
        mblock->blk_cnt--;
        if (p_err) {
            *p_err = OS_ERR_OK;
        }
        os_leave_protect(protect);
        mem = (void *)mem_link;
    } else if (ms < 0) {
        os_dbg("no mem block and no wait\n");
        if (p_err) {
            *p_err = OS_ERR_NONE;
        }
        os_leave_protect(protect);
    } else {
        os_leave_protect(protect);

        // 没有，则等待可用的资源
        os_err_t err = os_event_wait(&mblock->event, (void **)&mem, ms);
        if (p_err) {
            *p_err = err;
        }
    }

    // mem肯定要在连续的空间范围内
    os_assert((mem >= (mblock->mem)) && ((uint8_t *)mem < ((uint8_t *)mblock->mem + mblock->blk_max * mblock->blk_size)));
    return mem;
}

/**
 * 释放内存块
 * @param mblock 存储块
 * @param mem 内存块
 * @return 释放的结果
 */
os_err_t os_mblock_release (os_mblock_t * mblock, void * mem) {
    os_param_failed(mblock == OS_NULL, OS_ERR_PARAM);
    os_param_failed(mem == OS_NULL, OS_ERR_PARAM);
    os_param_failed(mem < mblock->mem, OS_ERR_PARAM);
    os_param_failed((uint8_t *)mem >= ((uint8_t *)mblock->mem + mblock->blk_max * mblock->blk_size), OS_ERR_PARAM);

    os_protect_t protect = os_enter_protect();
    os_task_t * task = os_event_wakeup(&mblock->event);
    if (!task) {
        // 没有任务在等待，将mem插入到空闲队列中
        mem_link_t * block = (mem_link_t *)mem;
        block->next = mblock->list;
        mblock->list = block;
        mblock->blk_cnt++;
        os_leave_protect(protect);
    } else {
        // 将mem交给对方，通过设置reason指向的消息
        os_wait_t * wait = task->wait;
        wait->err = OS_ERR_OK;
        *(void **)wait->reason = mem;
        task->wait = OS_NULL;

        os_leave_protect(protect);

        // 有任务等，通知调度器，看看是否要进行任务切换
        os_sched_run();
    }

    return OS_ERR_OK;
}
