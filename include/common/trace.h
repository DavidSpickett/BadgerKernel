#ifndef COMMON_TRACE_H
#define COMMON_TRACE_H

#include <stdint.h>
#include "thread.h"

#ifdef __aarch64__
#error
#elif defined __thumb__
#error
#else
typedef struct {
  size_t r0;
  size_t r1;
  size_t r2;
  size_t r3;
  size_t r4;
  size_t r5;
  size_t r6;
  size_t r7;
  size_t r8;
  size_t r9;
  size_t r10;
  size_t r11;
  size_t r12;
  /* stack pointer is in the thread struct */
  size_t lr;
  /* aka the exception mode lr */
  size_t pc;
  size_t cpsr;
  /* patched in by the kernel */
  size_t sp;
} __attribute__((packed)) RegisterContext;
// To tell the kernel that everything but sp needs
// to be written back to the thread's stack.
// (since sp is part of the Thread struct itself)
#define STACK_CTX_SIZE (sizeof(RegisterContext)-sizeof(size_t))
#endif

void print_register_context(RegisterContext ctx);

#endif /* ifdef COMMON_TRACE_H */
