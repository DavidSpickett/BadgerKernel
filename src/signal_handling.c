#include <string.h>
#include "thread.h"
#include "common/trace.h"

extern void __signal_handler_entry(void);
extern void __signal_handler_end(void);

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
#error
#else
  // Run in user mode
  ctx->cpsr = 0x10;
#endif
}

static void install_signal_handler(Thread* thread, uint32_t signal) {
	init_register_context(thread);
  RegisterContext* handler_ctx = (RegisterContext*)thread->stack_ptr;
  handler_ctx->pc = (size_t)__signal_handler_entry;
  // TODO: arch specific names
  handler_ctx->r0 = signal;
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
