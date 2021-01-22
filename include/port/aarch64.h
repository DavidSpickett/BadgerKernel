#ifndef PORT_AARCH64_H
#define PORT_AARCH64_H

#include "port/arm_common.h"
#include <stddef.h>

/* [[[cog
from scripts.registers import generate_context
generate_context("aarch64")
]]] */
typedef struct {
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
  union {
    size_t x20;
    size_t syscall_num;
  };
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
  union {
    size_t x3;
    size_t arg3;
  };
  size_t x4;
  size_t pc;
  union {
    size_t x2;
    size_t arg2;
  };
  size_t fpsr;
  size_t spsr_el1;
  union {
    size_t x0;
    size_t arg0;
  };
  union {
    size_t x1;
    size_t arg1;
  };
} __attribute__((packed)) RegisterContext;
/* [[[end]]] */

typedef struct {
  size_t x19;
  size_t x20;
  size_t x21;
  size_t x22;
  size_t x23;
  size_t x24;
  size_t x25;
  size_t x26;
  size_t x27;
  size_t x28;
  size_t x29;
  size_t lr; // aka x30
  size_t sp;
  size_t pc;
} __attribute__((packed)) FiberContext;

#define PC_ADD_MODE(pc)      (pc)
#define PC_REMOVE_MODE(pc)   (pc)
#define ALIGN_STACK_PTR(ptr) ((ptr) & ~0xF)

#endif /* ifdef PORT_AARCH64_H */
