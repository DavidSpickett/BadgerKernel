#ifndef PORT_ARM_H
#define PORT_ARM_H

#include "port/arm_common.h"
#include <stddef.h>

/* [[[cog
from scripts.registers import generate_context
generate_context("arm")
]]] */
typedef struct {
  union {
    size_t r0;
    size_t arg0;
  };
  union {
    size_t r1;
    size_t arg1;
  };
  union {
    size_t r2;
    size_t arg2;
  };
  union {
    size_t r3;
    size_t arg3;
  };
  size_t r4;
  size_t r5;
  size_t r6;
  size_t r7;
  union {
    size_t r8;
    size_t syscall_num;
  };
  size_t r9;
  size_t r10;
  size_t r11;
  size_t r12;
  size_t lr;
  size_t pc; // aka the exception mode lr
  size_t cpsr;
} __attribute__((packed)) RegisterContext;
/* [[[end]]] */

typedef struct {
  size_t r4;
  size_t r5;
  size_t r6;
  size_t r7;
  size_t r8;
  size_t r9;
  size_t r10;
  size_t r11;
  size_t sp;
  size_t lr;
  size_t pc;
} __attribute__((packed)) UserContext;

#define PC_ADD_MODE(pc)      (pc)
#define PC_REMOVE_MODE(pc)   (pc)
#define ALIGN_STACK_PTR(ptr) (ptr)

#endif /* ifdef PORT_ARM_H */
