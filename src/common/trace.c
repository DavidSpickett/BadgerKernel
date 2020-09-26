#include "common/trace.h"
#include "common/print.h"

#ifdef __aarch64__
void print_register_context(RegisterContext ctx) {
  PlatformRegs prs = ctx.platform_regs;
  printf("       x0: 0x%16x   x1: 0x%16x  x2: 0x%16x  x3: 0x%16x\n"
         "       x4: 0x%16x   x5: 0x%16x  x6: 0x%16x  x7: 0x%16x\n"
         "       x8: 0x%16x   x9: 0x%16x x10: 0x%16x x11: 0x%16x\n"
         "      x12: 0x%16x  x13: 0x%16x x14: 0x%16x x15: 0x%16x\n"
         "      x16: 0x%16x  x17: 0x%16x x18: 0x%16x x19: 0x%16x\n"
         "      x20: 0x%16x  x21: 0x%16x x22: 0x%16x x23: 0x%16x\n"
         "      x24: 0x%16x  x25: 0x%16x x26: 0x%16x x27: 0x%16x\n"
         "      x28: 0x%16x  x29: 0x%16x x30: 0x%16x  pc: 0x%16x\n"
         " spsr_el1: 0x%16x fpsr: 0x%16x\n",
         prs.x0, prs.x1, prs.x2, prs.x3, prs.x4, prs.x5, prs.x6, prs.x7, prs.x8,
         prs.x9, prs.x10, prs.x11, prs.x12, prs.x13, prs.x14, prs.x15, prs.x16,
         prs.x17, prs.x18, prs.x19, prs.x20, prs.x21, prs.x22, prs.x23, prs.x24,
         prs.x25, prs.x26, prs.x27, prs.x28, prs.x29, prs.x30, prs.pc,
         prs.spsr_el1, prs.fpsr);
}
#elif defined __thumb__
void print_register_context(RegisterContext ctx) {
  PlatformRegs prs = ctx.platform_regs;
  printf("  r0: 0x%08x r1: 0x%08x  r2: 0x%08x   r3: 0x%08x\n"
         "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x   r7: 0x%08x\n"
         "  r8: 0x%08x r9: 0x%08x r10: 0x%08x  r11: 0x%08x\n"
         " r12: 0x%08x lr: 0x%08x  pc: 0x%08x xpsr: 0x%08x\n",
         prs.r0, prs.r1, prs.r2, prs.r3, prs.r4, prs.r5, prs.r6, prs.r7, prs.r8,
         prs.r9, prs.r10, prs.r11, prs.r12, prs.lr, prs.pc, prs.xpsr);
}
#else
void print_register_context(RegisterContext ctx) {
  PlatformRegs prs = ctx.platform_regs;
  printf("  r0: 0x%08x r1: 0x%08x  r2: 0x%08x   r3: 0x%08x\n"
         "  r4: 0x%08x r5: 0x%08x  r6: 0x%08x   r7: 0x%08x\n"
         "  r8: 0x%08x r9: 0x%08x r10: 0x%08x  r11: 0x%08x\n"
         " r12: 0x%08x lr: 0x%08x  pc: 0x%08x cpsr: 0x%08x\n",
         prs.r0, prs.r1, prs.r2, prs.r3, prs.r4, prs.r5, prs.r6, prs.r7, prs.r8,
         prs.r9, prs.r10, prs.r11, prs.r12, prs.lr, prs.pc, prs.cpsr);
}
#endif

/* Backtraces work conistently for Arm with
   -mapcs-frame and mostly for AArch64.
   (give or take one frame because some functions
    don't modify the frame pointer)
   Thumb doesn't work at all, there doesn't
   seem to be a pattern to how the fp is handled.
*/

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

static const char* find_symbol(const Symbol* symbols, size_t num_symbols,
                               void* address) {
  if (!symbols) {
    return "???";
  }
  if (address == 0) {
    // All threads start with lr zeroed
    return "<init>";
  }

  for (size_t idx = 0; idx < num_symbols; ++idx) {
    if ((address >= symbols[idx].start) && (address < symbols[idx].end)) {
      return symbols[idx].name;
    }
  }
  return "???";
}

void print_backtrace(RegisterContext ctx, const Symbol* symbols,
                     size_t num_symbols) {
  FrameInfo info;
  PlatformRegs prs = ctx.platform_regs;
#ifdef __aarch64__
  info.fp = prs.x29;
  info.lr = prs.x30;
#else
  info.pc = prs.pc;
  info.lr = prs.lr;
  info.ip = prs.r12;
  info.fp = prs.r11;
#endif

  printf("0: 0x%08x (%s)\n", prs.pc,
         find_symbol(symbols, num_symbols, (void*)prs.pc));

  int depth = 1;
  while (info.fp) {
#ifdef __aarch64__
    // AArch64 points to stored fp
    info = *(FrameInfo*)(info.fp);
#else
    // Arm points to some way into the frame info
    info = *(FrameInfo*)(info.fp - 12);
#endif
    printf("%i: 0x%08x (%s)\n", depth, info.lr,
           find_symbol(symbols, num_symbols, (void*)info.lr));
    depth++;
  }
}
