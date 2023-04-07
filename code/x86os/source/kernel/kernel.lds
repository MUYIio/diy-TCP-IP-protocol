/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
SECTIONS
{
	PROVIDE(kernel_base = 0x0);

	. = 0x00010000;

	PROVIDE(s_text = .);
	.text : {
		*(EXCLUDE_FILE(*first_task* *net* *lib_syscall*) .text)
	}
	.rodata : {
		*(EXCLUDE_FILE(*first_task* *net* *lib_syscall*) .rodata)
	}
	PROVIDE(e_text = .);

	. = ALIGN(0x100000);
	PROVIDE(s_data = .);
	.data : {
		*(EXCLUDE_FILE(*first_task* *net* *lib_syscall*) .data)
	}
	.bss : {
		*(EXCLUDE_FILE(*first_task* *net*  *lib_syscall*) .bss)
	}
	e_data = .;

	/* 初始进程的配置：接紧着在低端1MB内存开始存储，但是运行时搬运到0x80000000处 */
	. = 0x80000000;

	PROVIDE(s_first_task = LOADADDR(.first_task));
	.first_task : AT(e_data) {
		*first_task_entry*(.text .data. rodata .data)
		*first_task*(.text .data. rodata .data)
	    *lib_syscall*(.text .data .bss)
	}
	PROVIDE(e_first_task = LOADADDR(.first_task) + SIZEOF(.first_task));

	PROVIDE(mem_free_start = e_first_task);
}
