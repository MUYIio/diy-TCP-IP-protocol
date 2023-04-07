#include "os_mem.h"
#include <stdlib.h>

void * os_mem_alloc (int size) {
    void * mem = malloc(size);
    return mem;
}

void os_mem_free (void * mem) {
    free(mem);
}

void os_mem_init (void) {

}

