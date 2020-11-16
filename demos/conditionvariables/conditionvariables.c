#include "common/assert.h"
#include "user/condition_variable.h"
#include "user/thread.h"

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
    yield();
  }

  // Signal the rest
  log_event("Broadcasting");
  broadcast(&cond_var);
  yield();

  // Should be false, no yield...
  bool signalled = signal(&cond_var);
  assert(!signalled);

  // Show that a new thread can wait after some have been signalled
  add_named_thread(final_signal, "final_signal");
  log_event("Waiting...");
  wait(&cond_var);
}

void setup(void) {
  init_condition_variable(&cond_var);

  const unsigned num_waiting = 5;
  for (unsigned i = 0; i < num_waiting; ++i) {
    add_thread_from_worker(waiter);
  }

  set_thread_name(CURRENT_THREAD, "signaller");
  signaller();
}
