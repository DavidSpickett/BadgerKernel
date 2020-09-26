#include "port/port.h"
#include "common/print.h"

void platform_init_register_context(RegisterContext* regs) {
  // Run in user mode
  regs->cpsr = 0x10;
}

void print_register_context(const RegisterContext* ctx) {
  printf("  r0: 0x%08x r1: 0x%08x  r2: 0x%08x   r3: 0x%08x\n"
         "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x   r7: 0x%08x\n"
         "  r8: 0x%08x r9: 0x%08x r10: 0x%08x  r11: 0x%08x\n"
         " r12: 0x%08x lr: 0x%08x  pc: 0x%08x cpsr: 0x%08x\n",
         ctx->r0, ctx->r1, ctx->r2, ctx->r3, ctx->r4, ctx->r5, ctx->r6, ctx->r7,
         ctx->r8, ctx->r9, ctx->r10, ctx->r11, ctx->r12, ctx->lr, ctx->pc,
         ctx->cpsr);
}
