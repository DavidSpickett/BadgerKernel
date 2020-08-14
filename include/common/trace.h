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
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t r12;
  /* stack pointer is in the thread struct */
  uint32_t lr;
  /* aka the exception mode lr */
  uint32_t pc;
  uint32_t cpsr;
  /* patched in by the kernel */
  uint32_t sp;
} __attribute__((packed)) RegisterContext;
// To tell the kernel that everything but sp needs
// to be written back to the thread's stack.
// (since sp is part of the Thread struct itself)
#define STACK_CTX_SIZE (sizeof(RegisterContext)-sizeof(uint32_t))
#endif

void print_register_context(RegisterContext ctx);

#endif /* ifdef COMMON_TRACE_H */
