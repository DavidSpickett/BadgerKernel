#include "user/alloc.h"
#include "common/syscall.h"

void* malloc(size_t size) {
  return (void*)DO_SYSCALL_1(malloc, size);
}

void* realloc(void* ptr, size_t size) {
  return (void*)DO_SYSCALL_2(realloc, ptr, size);
}

void free(void* ptr) {
  DO_SYSCALL_1(free, ptr);
}
