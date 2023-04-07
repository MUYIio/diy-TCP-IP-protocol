/**
 * 自己动手写操作系统
 *
 * 32位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "loader.h"
#include "comm/elf.h"

/**
* 使用LBA48位模式读取磁盘
*/
static void read_disk(int sector, int sector_count, uint8_t * buf) {
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_count >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

	// 读取数据
	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		// 每次扇区读之前都要检查，等待数据就绪
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// 读取并将数据写入到缓存中
		for (int i = 0; i < SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}

/**
 * 解析elf文件，提取内容到相应的内存中
 * https://wiki.osdev.org/ELF
 * @param file_buffer
 * @return
 */
static uint32_t reload_elf_file (uint8_t * file_buffer) {
    // 读取的只是ELF文件，不像BIN那样可直接运行，需要从中加载出有效数据和代码
    // 简单判断是否是合法的ELF文件
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    if ((elf_hdr->e_ident[0] != ELF_MAGIC) || (elf_hdr->e_ident[1] != 'E')
        || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')) {
        return 0;
    }

    // 然后从中加载程序头，将内容拷贝到相应的位置
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

		// 全部使用物理地址，此时分页机制还未打开
        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }

		// memsz和filesz不同时，后续要填0
		dest= (uint8_t *)phdr->p_paddr + phdr->p_filesz;
		for (int j = 0; j < phdr->p_memsz - phdr->p_filesz; j++) {
			*dest++ = 0;
		}
    }

    return elf_hdr->e_entry;
}

/**
 * 死机
 */
static void die (int code) {
    for (;;) {}
}


/**
 * @brief 开启分页机制
 * 将0-4M空间映射到0-4M和SYS_KERNEL_BASE_ADDR~+4MB空间
 * 0-4MB的映射主要用于保护loader自己还能正常工作
 * SYS_KERNEL_BASE_ADDR+4MB则用于为内核提供正确的虚拟地址空间
 */
void enable_page_mode (void) {
#define PDE_P			(1 << 0)
#define PDE_PS			(1 << 7)
#define PDE_W			(1 << 1)
#define CR4_PSE		    (1 << 4)
#define CR0_PG		    (1 << 31)

    // 使用4MB页块，这样构造页表就简单很多，只需要1个表即可。
    // 以下表为临时使用，用于帮助内核正常运行，在内核运行起来之后，将重新设置
    static uint32_t page_dir[1024] __attribute__((aligned(4096))) = {
        [0] = PDE_P | PDE_PS | PDE_W,			// PDE_PS，开启4MB的页
    };

    // 设置PSE，以便启用4M的页，而不是4KB
    uint32_t cr4 = read_cr4();
    write_cr4(cr4 | CR4_PSE);

    // 设置页表地址
    write_cr3((uint32_t)page_dir);

    // 开启分页机制
    write_cr0(read_cr0() | CR0_PG);
}

/**
 * 从磁盘上加载内核
 */
void load_kernel(void) {
    // 读取的扇区数一定要大一些，保不准kernel.elf大小会变得很大
    // 我就吃过亏，只读了100个扇区，结果运行后发现kernel的一些初始化的变量值为空，程序也会跑飞
    read_disk(100, 1000, (uint8_t *)SYS_KERNEL_LOAD_ADDR);

     // 解析ELF文件，并通过调用的方式，进入到内核中去执行，同时传递boot参数
	 // 临时将elf文件先读到SYS_KERNEL_LOAD_ADDR处，再进行解析
    uint32_t kernel_entry = reload_elf_file((uint8_t *)SYS_KERNEL_LOAD_ADDR);
	if (kernel_entry == 0) {
		die(-1);
	}

	// 开启分页机制
	enable_page_mode();

    ((void (*)(boot_info_t *))kernel_entry)(&boot_info);
    for (;;) {}
}