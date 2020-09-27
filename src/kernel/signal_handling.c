#include "common/attribute.h"
#include "common/trace.h"
#include "kernel/thread.h"
#include "port/port.h"
#include <string.h>

void init_register_context(Thread* thread) {
  // This sets up the initial entry state of a thread
  // which will be restored on first run.
  // Adding this now allows us to assume all threads
  // have a full register context regardless of state.

  thread->stack_ptr -= sizeof(RegisterContext);
  RegisterContext* ctx = (RegisterContext*)thread->stack_ptr;
  memset(ctx, 0, sizeof(RegisterContext));

  ctx->pc = (size_t)thread_start;

  // Set arch specific settings registers
  platform_init_register_context(ctx);
}

static void install_signal_handler(Thread* thread, uint32_t signal) {
  init_register_context(thread);
  RegisterContext* handler_ctx = (RegisterContext*)thread->stack_ptr;
  handler_ctx->pc = (size_t)signal_handler_wrapper;
  handler_ctx->arg0 = signal;
  handler_ctx->arg1 = (size_t)thread->signal_handler;
}

static uint32_t next_signal(uint32_t pending_signals) {
  for (uint32_t i = 0; i < 32; ++i) {
    if (pending_signals & ((uint32_t)1 << i)) {
      // +1 because bit 0 means signal number 1
      return i + 1;
    }
  }
  return 0;
}

void check_signals(Thread* thread) {
  const RegisterContext* next_ctx = (const RegisterContext*)thread->stack_ptr;

  if (next_ctx->pc == PC_REMOVE_MODE((size_t)signal_handler_wrapper_end)) {
    // Remove signal handler context
    thread->stack_ptr += sizeof(RegisterContext);
  }

  if (thread->pending_signals) {
    uint32_t signal = next_signal(thread->pending_signals);

    if (thread->signal_handler) {
      install_signal_handler(thread, signal);
      // Mark signal as handled
      thread->pending_signals &= ~(1 << (signal - 1));
    } else {
      // Ignore all signals if there's no handler
      thread->pending_signals = 0;
    }
  }
}
