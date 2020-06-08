#ifndef USER_ALLOC_H
#define USER_ALLOC_H

#include <stddef.h>

void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
void free_all(int tid);

#endif /* ifdef USER_ALLOC_H */
