#ifndef PORT_AARCH64_H
#define PORT_AARCH64_H

#include "port/arm_common.h"
#include <stddef.h>

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
  /* stack pointer is in thread struct
     We don't provide it here because modifying it
     would prevent the thread from resuming correctly.
  */
} __attribute__((packed)) RegisterContext;

#endif /* ifdef PORT_AARCH64_H */
