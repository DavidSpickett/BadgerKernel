#ifndef ALLOC_H
#define ALLOC_H

#ifdef linux
#include <stdlib.h>
#else
#include <stddef.h>

void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
void free_all(int tid);
#endif

#endif /* ifdef ALLOC_H */
