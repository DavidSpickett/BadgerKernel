#include "util.h"

void* memcpy (void* destination, const void* source, size_t num) {
  for ( ; num > 0; --num) {
    *(char*)destination++ = *(char*)source++;
  }
  return destination;
}

void *memset(void *str, int c, size_t n) {
  int* mem = (int*)str;
  for( ; n; ++mem, --n) {
    *mem = c;
  }
  return str;
}

size_t strlen(const char *str) {
  if (str == NULL) {
    return 0;
  }

  size_t len = 0;
  while (*str != '\0') { ++str; ++len; }
  return len;
}

char *strncpy(char *dest, const char *src, size_t n) {
  if ((dest == NULL) || (src == NULL)) {
    return dest;
  }

  for( ; *src != '\0' && n ; ++src, ++dest, --n) {
    *dest = *src;
  }

  for( ; n; ++dest, --n) {
    *dest = '\0';
  }

  return dest;
}
