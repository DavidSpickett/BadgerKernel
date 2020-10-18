#include "port/port.h"
#include "common/print.h"

void platform_init_register_context(RegisterContext* regs) {
  // Run in user mode
  regs->cpsr = 0x10;
}

void print_register_context(const RegisterContext* ctx) {
  printf("  r0: 0x%08X r1: 0x%08X  r2: 0x%08X   r3: 0x%08X\n"
         "  r4: 0x%08X r5: 0x%08X  r6: 0x%08X   r7: 0x%08X\n"
         "  r8: 0x%08X r9: 0x%08X r10: 0x%08X  r11: 0x%08X\n"
         " r12: 0x%08X lr: 0x%08X  pc: 0x%08X cpsr: 0x%08X\n",
         ctx->r0, ctx->r1, ctx->r2, ctx->r3, ctx->r4, ctx->r5, ctx->r6, ctx->r7,
         ctx->r8, ctx->r9, ctx->r10, ctx->r11, ctx->r12, ctx->lr, ctx->pc,
         ctx->cpsr);
}
