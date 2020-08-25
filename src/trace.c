#include "common/trace.h"
#include "print.h"

// TODO: test these layouts somehow
#ifdef __aarch64__
void print_register_context(RegisterContext ctx) {
  printf(
  "       x0: 0x%16x   x1: 0x%16x  x2: 0x%16x  x3: 0x%16x\n"
  "       x4: 0x%16x   x5: 0x%16x  x6: 0x%16x  x7: 0x%16x\n"
  "       x8: 0x%16x   x9: 0x%16x x10: 0x%16x x11: 0x%16x\n"
  "      x12: 0x%16x  x13: 0x%16x x14: 0x%16x x15: 0x%16x\n"
  "      x16: 0x%16x  x17: 0x%16x x18: 0x%16x x19: 0x%16x\n"
  "      x20: 0x%16x  x21: 0x%16x x22: 0x%16x x23: 0x%16x\n"
  "      x24: 0x%16x  x25: 0x%16x x26: 0x%16x x27: 0x%16x\n"
  "      x28: 0x%16x  x29: 0x%16x x30: 0x%16x  pc: 0x%16x\n"
  " spsr_el1: 0x%16x fpsr: 0x%16x\n",
  ctx.x0,  ctx.x1,  ctx.x2,  ctx.x3,
  ctx.x4,  ctx.x5,  ctx.x6,  ctx.x7,
  ctx.x8,  ctx.x9,  ctx.x10, ctx.x11,
  ctx.x12, ctx.x13, ctx.x14, ctx.x15,
  ctx.x16, ctx.x17, ctx.x18, ctx.x19,
  ctx.x20, ctx.x21, ctx.x22, ctx.x23,
  ctx.x24, ctx.x25, ctx.x26, ctx.x27,
  ctx.x28, ctx.x29, ctx.x30, ctx.pc,
  ctx.spsr_el1, ctx.fpsr);
}
#elif defined __thumb__
void print_register_context(RegisterContext ctx) {
  printf(
  "  r0: 0x%08x r1: 0x%08x  r2: 0x%08x   r3: 0x%08x\n"
  "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x   r7: 0x%08x\n"
  "  r8: 0x%08x r9: 0x%08x r10: 0x%08x  r11: 0x%08x\n"
  " r12: 0x%08x lr: 0x%08x  pc: 0x%08x xpsr: 0x%08x\n",
  ctx.r0,  ctx.r1, ctx.r2,  ctx.r3,
  ctx.r4,  ctx.r5, ctx.r6,  ctx.r7,
  ctx.r8,  ctx.r9, ctx.r10, ctx.r11,
  ctx.r12, ctx.lr, ctx.pc,  ctx.xpsr);
}
#else
void print_register_context(RegisterContext ctx) {
  printf(
  "  r0: 0x%08x r1: 0x%08x  r2: 0x%08x   r3: 0x%08x\n"
  "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x   r7: 0x%08x\n"
  "  r8: 0x%08x r9: 0x%08x r10: 0x%08x  r11: 0x%08x\n"
  " r12: 0x%08x lr: 0x%08x  pc: 0x%08x cpsr: 0x%08x\n",
  ctx.r0,  ctx.r1, ctx.r2,  ctx.r3,
  ctx.r4,  ctx.r5, ctx.r6,  ctx.r7,
  ctx.r8,  ctx.r9, ctx.r10, ctx.r11,
  ctx.r12, ctx.lr, ctx.pc,  ctx.cpsr);
}
#endif

/* All of this backtrace stuff is assuming that
   -mapcs-frame is used. Even then only on Arm.
   Thumb appears to have no standard.
   AArch64 works but will show a different number
   of frames due to some functions not modifying
   the frame pointer. */

typedef struct {
#ifdef __aarch64__
  size_t fp;
  size_t lr;
#else
  size_t fp;
  size_t ip;
  size_t lr;
  size_t pc;
#endif
} FrameInfo;

static const char* find_symbol(const Symbol* symbols,
                               size_t num_symbols,
                               void* address) {
  if (address == 0) {
    // All threads start with lr zeroed
    return "<init>";
  }

  for (size_t idx=0; idx<num_symbols; ++idx) {
    if (
      (address >= symbols[idx].start) &&
      (address <  symbols[idx].end)
    ) {
      return symbols[idx].name;
    }
  }
  return "???";
}

void print_backtrace(RegisterContext ctx,
                     const Symbol* symbols,
                     size_t num_symbols) {
  FrameInfo info;
#ifdef __aarch64__
  info.fp = ctx.x29;
  info.lr = ctx.x30;
#else
  info.pc = ctx.pc;
  info.lr = ctx.lr;
  info.ip = ctx.r12;
  info.fp = ctx.r11;
#endif

  printf("0: 0x%08x (%s)\n", ctx.pc,
      find_symbol(symbols, num_symbols, (void*)ctx.pc));

  int depth = 1;
  while (info.fp) {
#ifdef __aarch64__
    // AArch64 points to stored fp
    info = *(FrameInfo*)(info.fp);
#else
    // Arm points to some way into the frame info
    info = *(FrameInfo*)(info.fp-12);
#endif
    printf("%i: 0x%08x (%s)\n", depth, info.lr,
      find_symbol(symbols, num_symbols, (void*)info.lr));
    depth++;
  }
}
