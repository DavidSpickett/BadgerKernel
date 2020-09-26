#include "port/port.h"
#include "common/print.h"

void platform_init_register_context(RegisterContext* regs) {
  // Run in EL0
  regs->spsr_el1 = 0;
}

void print_register_context(const RegisterContext* ctx) {
  printf("       x0: 0x%16x   x1: 0x%16x  x2: 0x%16x  x3: 0x%16x\n"
         "       x4: 0x%16x   x5: 0x%16x  x6: 0x%16x  x7: 0x%16x\n"
         "       x8: 0x%16x   x9: 0x%16x x10: 0x%16x x11: 0x%16x\n"
         "      x12: 0x%16x  x13: 0x%16x x14: 0x%16x x15: 0x%16x\n"
         "      x16: 0x%16x  x17: 0x%16x x18: 0x%16x x19: 0x%16x\n"
         "      x20: 0x%16x  x21: 0x%16x x22: 0x%16x x23: 0x%16x\n"
         "      x24: 0x%16x  x25: 0x%16x x26: 0x%16x x27: 0x%16x\n"
         "      x28: 0x%16x  x29: 0x%16x x30: 0x%16x  pc: 0x%16x\n"
         " spsr_el1: 0x%16x fpsr: 0x%16x\n",
         ctx->x0, ctx->x1, ctx->x2, ctx->x3, ctx->x4, ctx->x5, ctx->x6, ctx->x7,
         ctx->x8, ctx->x9, ctx->x10, ctx->x11, ctx->x12, ctx->x13, ctx->x14,
         ctx->x15, ctx->x16, ctx->x17, ctx->x18, ctx->x19, ctx->x20, ctx->x21,
         ctx->x22, ctx->x23, ctx->x24, ctx->x25, ctx->x26, ctx->x27, ctx->x28,
         ctx->x29, ctx->x30, ctx->pc, ctx->spsr_el1, ctx->fpsr);
}
