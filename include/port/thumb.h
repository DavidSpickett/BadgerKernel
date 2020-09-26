#ifndef PORT_THUMB_H
#define PORT_THUMB_H

#include "port/arm_common.h"
#include <stddef.h>

typedef struct {
  size_t r4;
  size_t r5;
  size_t r6;
  size_t r7;
  size_t r8;
  size_t r9;
  size_t r10;
  size_t r11;
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
  size_t r12;
  size_t lr;
  size_t pc;
  size_t xpsr;
} __attribute__((packed)) RegisterContext;

#define PC_ADD_MODE(pc)    ((pc) | 1)
#define PC_REMOVE_MODE(pc) ((pc) & ~1)

#endif /* ifdef PORT_THUMB_H */
