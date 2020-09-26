#include "common/attribute.h"
#include "common/generic_asm.h"
#include "common/trace.h"
#include "kernel/thread.h"
#include <string.h>

extern void __signal_handler_entry(void);
extern void __signal_handler_end(void);

ATTR_NAKED __attribute__((used)) static void handler_wrapper(void) {
  asm volatile(".global __signal_handler_entry\n\t"
               "__signal_handler_entry:\n\t"
               // r0/x0 has the signal number
               "  ldr " RCHR "1, =_current_thread\n\t"
               "  ldr " RCHR "1, [" RCHR "1]\n\t"
               "  add " RCHR "1, " RCHR "1, #2*" PTR_SIZE "\n\t"
               "  ldr " RCHR "1, [" RCHR "1]\n\t"
               "  " BLR " " RCHR "1\n\t"
               "  svc %0\n\t"
               ".global __signal_handler_end\n\t"
               "__signal_handler_end:\n\t"
               "  nop\n\t" ::"i"(svc_thread_switch));
}

void init_register_context(Thread* thread) {
  // This sets up the initial entry state of a thread
  // which will be restored on first run.
  // Adding this now allows us to assume all threads
  // have a full register context regardless of state.

  thread->stack_ptr -= sizeof(RegisterContext);
  RegisterContext* ctx = (RegisterContext*)thread->stack_ptr;
  memset(ctx, 0, sizeof(RegisterContext));

  ctx->generic_regs.pc = (size_t)thread_start;

  // Set arch specific settings registers
  platform_init_register_context(&ctx->platform_regs);
}

static void install_signal_handler(Thread* thread, uint32_t signal) {
  init_register_context(thread);
  RegisterContext* handler_ctx = (RegisterContext*)thread->stack_ptr;
  handler_ctx->generic_regs.pc = (size_t)__signal_handler_entry;
  handler_ctx->generic_regs.arg0 = signal;
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

  // Stored thread PC doesn't include Thumb bit
  if (next_ctx->generic_regs.pc == ((size_t)__signal_handler_end & ~1)) {
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
