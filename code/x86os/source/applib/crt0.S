/**
 * 进程启动汇编部分代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
     .text
    .global _start
    .extern cstart
_start:
    # 设置各数据段的选择子,由于应用任务都是用tss恢复的，所以
    # 实际不必设置，但为安全起见，还是设置一下
    mov %ss, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # 进入主函数执行，注意此时栈没有变，参数仍然在其中
    # 所以调用cstart时，仍然可以看到参数
    jmp cstart
