#include "thread.h"
#include "condition_variable.h"

ConditionVariable cv;

void waiter(void) {
  log_event("Waiting...");
  wait(&cv);
  log_event("Signalled");
}

void final_signal(void) {
  log_event("Signalling");
  signal(&cv);
}

void signaller(void) {
  // Yields back to us, as others are waiting
  yield();

  // Signal a few individually
  for (unsigned i=0; i<2; ++i) {
    log_event("Signalling");
    signal(&cv);
    yield_next();
  }

  // Signal the rest
  log_event("Broadcasting");
  broadcast(&cv);
  yield();

  // Should be false, no yield...
  if (signal(&cv)) {
    yield();
  }

  // Show that a new thread can wait after some have been signalled
  add_named_thread(final_signal, "final_signal");
  log_event("Waiting...");
  wait(&cv);
}

void demo() {
  init_condition_variable(&cv);

  const unsigned num_waiting = 5;
  for (unsigned i=0; i < num_waiting; ++i) {
    add_thread(waiter);
  }
  add_named_thread(signaller, "signaller");

  start_scheduler();
}
