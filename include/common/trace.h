#ifndef COMMON_TRACE_H
#define COMMON_TRACE_H

#include <stdint.h>
#include "thread.h"

typedef struct {
#ifdef __aarch64__
  size_t x29;
  size_t x30; // aka lr
  size_t x27;
  size_t x28;
  size_t x25;
  size_t x26;
  size_t x23;
  size_t x24;
  size_t x21;
  size_t x22;
  size_t x19;
  size_t x20;
  size_t x17;
  size_t x18;
  size_t x15;
  size_t x16;
  size_t x13;
  size_t x14;
  size_t x11;
  size_t x12;
  size_t x9;
  size_t x10;
  size_t x7;
  size_t x8;
  size_t x5;
  size_t x6;
  size_t x3;
  size_t x4;
  size_t pc;
  size_t x2;
  size_t fpsr;
  size_t spsr_el1;
  size_t x0;
  size_t x1;
#elif defined __thumb__
  size_t r4;
  size_t r5;
  size_t r6;
  size_t r7;
  size_t r8;
  size_t r9;
  size_t r10;
  size_t r11;
  size_t r0;
  size_t r1;
  size_t r2;
  size_t r3;
  size_t r12;
  size_t lr;
  size_t pc;
  size_t xpsr;
#else
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
#endif
  /* stack pointer is in thread struct
     We don't provide it here because modifying it
     would prevent the thread from resuming correctly.
  */
} __attribute__((packed)) RegisterContext;

void print_register_context(RegisterContext ctx);

typedef struct {
  const char* name;
  void* start;
  void* end;
} Symbol;

void print_backtrace(RegisterContext ctx,
  const Symbol* symbols, size_t num_symbols);

#endif /* ifdef COMMON_TRACE_H */
