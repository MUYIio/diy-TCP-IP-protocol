#ifndef OS_MEM_H
#define OS_MEM_H

void os_mem_init (void);
void * os_mem_alloc (int size);
void os_mem_free (void * mem);

#endif // OS_MEM_H

