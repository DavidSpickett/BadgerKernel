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
  size_t _p8_;
  size_t _p9_;
  size_t _p10_;
  size_t _p11_;
  size_t _p12_;
  size_t _p13_;
  size_t _p14_;
  size_t _p15_;
  size_t _p16_;
  size_t _p17_;
  size_t _p18_;
  size_t _p19_;
  size_t _p20_;
  size_t _p21_;
  size_t _p22_;
  size_t _p23_;
  size_t _p24_;
  size_t _p25_;
  size_t arg3;
  size_t _p26_;
  size_t pc;
  size_t arg2;
  size_t _p27_;
  size_t _p28_;
  size_t arg0;
  size_t arg1;
} __attribute__((packed)) GenericRegs;

#define NEXT_INSTR(pc)     (pc + 4)
#define PC_ADD_MODE(pc)    (pc)
#define PC_REMOVE_MODE(pc) (pc)

#endif /* ifdef PORT_AARCH64_H */
