#include <stddef.h>

void* memcpy (void* destination, const void* source, size_t num) {
  for ( ; num > 0; --num) {
    *(char*)destination++ = *(char*)source++;
  }
  return destination;
}
