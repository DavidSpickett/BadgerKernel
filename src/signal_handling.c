#include <string.h>
#include "thread.h"
#include "common/trace.h"

extern void __signal_handler_entry(void);
extern void __signal_handler_end(void);

// TODO: generic asm header?
#ifdef __aarch64__
#define RCHR "x"
#define OFFSET "16"
#define BLR "blr"
#else
#define RCHR "r"
#define OFFSET "8"
#define BLR "blx"
#endif

__attribute__((used, naked))
static void handler_wrapper(void) {
  asm volatile (
    ".global __signal_handler_entry\n\t"
    "__signal_handler_entry:\n\t"
    // r0/x0 has the signal number
    "  ldr "RCHR"1, =_current_thread\n\t"
    "  ldr "RCHR"1, ["RCHR"1]\n\t"
    "  add "RCHR"1, "RCHR"1, #"OFFSET"\n\t"
    "  ldr "RCHR"1, ["RCHR"1]\n\t"
    "  "BLR" "RCHR"1\n\t"
    "  svc %0\n\t"
    ".global __signal_handler_end\n\t"
    "__signal_handler_end:\n\t"
    "  nop\n\t"
    :: "i"(svc_thread_switch));
}

void init_register_context(Thread* thread) {
  // This sets up the initial entry state of a thread
  // which will be restored on first run.
  // Adding this now allows us to assume all threads
  // have a full register context regardless of state.

  thread->stack_ptr -= sizeof(RegisterContext);
  RegisterContext* ctx = (RegisterContext*)thread->stack_ptr;
  memset(ctx, 0, sizeof(RegisterContext));

  ctx->pc = (size_t)thread_start;

#ifdef __thumb__
  // Must run in Thumb mode
  ctx->xpsr = (1<<24);
#elif defined __aarch64__
  // Run in EL0
  ctx->spsr_el1 = 0;
#else
  // Run in user mode
  ctx->cpsr = 0x10;
#endif
}

static void install_signal_handler(Thread* thread, uint32_t signal) {
	init_register_context(thread);
  RegisterContext* handler_ctx = (RegisterContext*)thread->stack_ptr;
  handler_ctx->pc = (size_t)__signal_handler_entry;
#ifdef __aarch64__
  handler_ctx->x0 = signal;
#else
  handler_ctx->r0 = signal;
#endif
}

static uint32_t next_signal(uint32_t pending_signals) {
  for (uint32_t i=0; i<32; ++i) {
    if (pending_signals & (1<<i)) {
      // +1 because bit 0 means signal number 1
      return i+1;
    }
  }
  return 0;
}

void check_signals(Thread* thread) {
  const RegisterContext* next_ctx =
    (const RegisterContext*)thread->stack_ptr;

  // Stored PC doesn't include Thumb bit
  if (next_ctx->pc == ((size_t)__signal_handler_end & ~1  )) {
    // Remove signal handler context
    thread->stack_ptr += sizeof(RegisterContext);
  }

  if (thread->pending_signals) {
    uint32_t signal = next_signal(thread->pending_signals);

    if (thread->signal_handler) {
      install_signal_handler(thread, signal);
      // Mark signal as handled
      thread->pending_signals &= ~(1 << (signal-1));
    } else {
      // Ignore all signals if there's no handler
      thread->pending_signals = 0;
    }
  }
}
