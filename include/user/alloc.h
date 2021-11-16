#ifndef USER_ALLOC_H
#define USER_ALLOC_H

#include "common/macros.h"
#include <stddef.h>

BK_EXPORT void* malloc(size_t size);
BK_EXPORT void* realloc(void* ptr, size_t size);
BK_EXPORT void free(void* ptr);

#endif /* ifdef USER_ALLOC_H */
