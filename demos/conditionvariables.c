#include "condition_variable.h"
#include "thread.h"

ConditionVariable cond_var;

void waiter(void) {
  log_event("Waiting...");
  wait(&cond_var);
  log_event("Signalled");
}

void final_signal(void) {
  log_event("Signalling");
  signal(&cond_var);
}

void signaller(void) {
  // Yields back to us, as others are waiting
  yield();

  // Signal a few individually
  for (unsigned i = 0; i < 2; ++i) {
    log_event("Signalling");
    signal(&cond_var);
    yield_next();
  }

  // Signal the rest
  log_event("Broadcasting");
  broadcast(&cond_var);
  yield();

  // Should be false, no yield...
  if (signal(&cond_var)) {
    yield();
  }

  // Show that a new thread can wait after some have been signalled
  add_named_thread(final_signal, "final_signal");
  log_event("Waiting...");
  wait(&cond_var);
}

void setup(void) {
  config.log_scheduler = false;

  init_condition_variable(&cond_var);

  const unsigned num_waiting = 5;
  for (unsigned i = 0; i < num_waiting; ++i) {
    add_thread(waiter);
  }
  add_named_thread(signaller, "signaller");
}
