#ifndef KERNEL_ALLOC_H
#define KERNEL_ALLOC_H

#include <stddef.h>

void* k_malloc(size_t size);
void* k_realloc(void* ptr, size_t size);
void k_free(void* ptr);
void k_free_all(int tid);

#endif /* ifdef KERNEL_ALLOC_H */
