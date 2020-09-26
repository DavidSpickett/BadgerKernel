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
  size_t r0;
  size_t r1;
  size_t r2;
  size_t r3;
  size_t r12;
  size_t lr;
  size_t pc;
  size_t xpsr;
} __attribute__((packed)) PlatformRegs;

typedef struct {
  size_t _p0_;
  size_t _p1_;
  size_t _p2_;
  size_t _p3_;
  size_t _p4_;
  size_t _p5_;
  size_t _p6_;
  size_t _p7_;
  size_t arg0;
  size_t arg1;
  size_t arg2;
  size_t arg3;
  size_t _p8_;
  size_t _p9_;
  size_t pc;
  size_t _p10_;
} __attribute__((packed)) GenericRegs;

#endif /* ifdef PORT_THUMB_H */
