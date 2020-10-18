#include "common/trace.h"
#include "common/print.h"

/* Backtraces only work conistently for Arm with
   -mapcs-frame.
   AArch64 is mostly consistent but still breaks.
   Thumb doesn't work at all, there doesn't
   seem to be a pattern to how the fp is handled.
*/

#if defined __arm__ && !defined __thumb__
typedef struct {
  size_t fp;
  size_t ip;
  size_t lr;
  size_t pc;
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
  info.pc = ctx.pc;
  info.lr = ctx.lr;
  info.ip = ctx.r12;
  info.fp = ctx.r11;

  printf("0: 0x%08X (%s)\n", ctx.pc,
         find_symbol(symbols, num_symbols, (void*)ctx.pc));

  int depth = 1;
  while (info.fp) {
    // Arm points to some way into the frame info
    info = *(FrameInfo*)(info.fp - 12);
    printf("%i: 0x%08X (%s)\n", depth, info.lr,
           find_symbol(symbols, num_symbols, (void*)info.lr));
    depth++;
  }
}
#endif
