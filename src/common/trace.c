#include "common/trace.h"
#include "common/print.h"

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
    info = *(FrameInfo*)(info.fp - 12);
#endif
    printf("%i: 0x%08x (%s)\n", depth, info.lr,
           find_symbol(symbols, num_symbols, (void*)info.lr));
    depth++;
  }
}
