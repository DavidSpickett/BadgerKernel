#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void* memcpy (void* destination, const void* source, size_t num);
void *memset(void *str, int c, size_t n);
size_t strlen(const char *str);
char *strncpy(char *dest, const char *src, size_t n);

#endif /* ifdef UTIL_H */
