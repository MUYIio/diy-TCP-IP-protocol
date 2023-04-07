/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "core/task.h"
#include "os_cfg.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "ipc/sem.h"
#include "core/memory.h"
#include "dev/console.h"
#include "dev/kbd.h"
#include "fs/fs.h"
#include "net.h"

static boot_info_t * init_boot_info;        // 启动信息

#include "netif.h"
#include "rtl8139.h"


static rtl8139_priv_t priv1, priv2;

/**
 * @brief 网络设备初始化
 */
net_err_t netdev_init(void) {
    // 打开网络接口
    netif_t* netif = netif_open("netif 0", &netdev8139_ops, &priv1);
    if (!netif) {
        log_printf("netif %d open failed.", 0);
        return NET_ERR_IO;
    }

    // 生成相应的地址
    ipaddr_t ip, mask, gw;
    ipaddr_from_str(&ip, netdev0_ip);
    ipaddr_from_str(&mask, netdev0_mask);
    ipaddr_from_str(&gw, netdev0_gw);
    netif_set_addr(netif, &ip, &mask, &gw);
    netif_set_active(netif);

    // 打开网络接口2
    netif = netif_open("netif 1", &netdev8139_ops, &priv2);
    if (!netif) {
        log_printf("netif %d open failed.", 1);
        return NET_ERR_IO;
    }
    ipaddr_from_str(&ip, netdev1_ip);
    ipaddr_from_str(&mask, netdev1_mask);
    ipaddr_from_str(&gw, netdev1_gw);
    netif_set_addr(netif, &ip, &mask, &gw);
    netif_set_active(netif);

    // 使用接口2作为缺省的接口
    netif_set_default(netif);
    return NET_ERR_OK;
}

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;

    // 初始化CPU，再重新加载
    cpu_init();
    irq_init();
    log_init();

    // 内存初始化要放前面一点，因为后面的代码可能需要内存分配
    memory_init(boot_info);
    fs_init();

    time_init();

    task_manager_init();

    // 所有其它模块初始化完成后，再初始化网络部分
    net_init();

    // 初始网络接口
    netdev_init();
}


/**
 * @brief 移至第一个进程运行
 */
void move_to_first_task(void) {
    // 不能直接用Jmp far进入，因为当前特权级0，不能跳到低特权级的代码
    // 下面的iret后，还需要手动加载ds, fs, es等寄存器值，iret不会自动加载
    // 注意，运行下面的代码可能会产生异常：段保护异常或页保护异常。
    // 可根据产生的异常类型和错误码，并结合手册来找到问题所在
    task_t * curr = task_current();
    ASSERT(curr != 0);

    tss_t * tss = &(curr->tss);

    // 也可以使用类似boot跳loader中的函数指针跳转
    // 这里用jmp是因为后续需要使用内联汇编添加其它代码
    __asm__ __volatile__(
        // 模拟中断返回，切换入第1个可运行应用进程
        // 不过这里并不直接进入到进程的入口，而是先设置好段寄存器，再跳过去
        "push %[ss]\n\t"			// SS
        "push %[esp]\n\t"			// ESP
        "push %[eflags]\n\t"           // EFLAGS
        "push %[cs]\n\t"			// CS
        "push %[eip]\n\t"		    // ip
        "iret\n\t"::[ss]"r"(tss->ss),  [esp]"r"(tss->esp), [eflags]"r"(tss->eflags),
        [cs]"r"(tss->cs), [eip]"r"(tss->eip));
}

void init_main(void) {
    log_printf("==============================");
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("==============================");

    // 初始化任务
    net_start();
    task_first_init();
    move_to_first_task();
}
