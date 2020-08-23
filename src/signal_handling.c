#include <string.h>
#include "thread.h"
#include "common/trace.h"

extern void __signal_handler_entry(void);
extern void __signal_handler_end(void);

void install_signal_handler(Thread* thread, unsigned int signal) {
  thread->stack_ptr -= sizeof(RegisterContext);
  RegisterContext* handler_ctx =
		(RegisterContext*)thread->stack_ptr;
  memset(handler_ctx, 0, sizeof(RegisterContext));

  handler_ctx->pc = (size_t)__signal_handler_entry;
  // TODO: arch specific names
  handler_ctx->r0 = signal;
  // TODO: thumb specific
  // Run in Thumb mode
#ifdef __thumb__
  handler_ctx->xpsr = (1<<24);
#endif
}

void check_signals(Thread* thread) {
  const RegisterContext* next_ctx =
    (const RegisterContext*)thread->stack_ptr;

  // Stored PC doesn't include Thumb bit
  if (next_ctx->pc == ((size_t)__signal_handler_end & ~1  )) {

    // Finish handling
    thread->pending_signal = 0;
    // Remove signal handler context
    thread->stack_ptr += sizeof(RegisterContext);
  }

  if (thread->pending_signal) {
    if (thread->signal_handler) {
      install_signal_handler(thread,
        thread->pending_signal);
    } else {
      thread->pending_signal = 0;
    }
  }
}
