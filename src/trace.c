#include "common/trace.h"
#include "print.h"

#ifdef __aarch64__
#error
#elif defined __thumb__
#error
#else
void print_register_context(RegisterContext ctx) {
  printf(
  "  r0: 0x%08x r1: 0x%08x  r2: 0x%08x  r3: 0x%08x\n"
  "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x  r7: 0x%08x\n"
  "  r8: 0x%08x r9: 0x%08x r10: 0x%08x r11: 0x%08x\n"
  " r12: 0x%08x sp: 0x????????  lr: 0x%08x  pc: 0x%08x\n"
  "cpsr: 0x%08x\n",
  ctx.r0, ctx.r1, ctx.r2,  ctx.r3,
  ctx.r4, ctx.r5, ctx.r6,  ctx.r7,
  ctx.r8, ctx.r9, ctx.r10, ctx.r11,
  ctx.r12, /* stack ptr goes here */ ctx.lr, ctx.pc,
  ctx.cpsr);
}
#endif
