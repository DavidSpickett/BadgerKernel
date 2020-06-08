#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void* k_malloc(size_t size);
void* k_realloc(void* ptr, size_t size);
void k_free(void* ptr);
void k_free_all(int tid);

#endif /* ifdef ALLOC_H */
