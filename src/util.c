#include "util.h"

void* memcpy (void* destination, const void* source, size_t num) {
  while (num--) {
    *(unsigned char*)destination++ = *(unsigned char*)source++;
  }
  return destination;
}

void *memset(void *str, int c, size_t n) {
  unsigned char* mem = (unsigned char*)str;
  while(n--) {
    *mem++ = (unsigned char)c;
  }
  return str;
}

size_t strlen(const char *str) {
  size_t len = 0;
  while (*str != '\0') { ++str; ++len; }
  return len;
}

char *strncpy(char *dest, const char *src, size_t n) {
  for( ; *src != '\0' && n ; ++src, ++dest, --n) {
    *dest = *src;
  }

  for( ; n; ++dest, --n) {
    *dest = '\0';
  }

  return dest;
}
