#include "user/thread.h"
#include "user/util.h"

void signal_handler(void) {
  log_event("got a signal!");
}

void other_signal_handler(void) {
  log_event("got a signal again!");
}

void worker() {
  log_event("hello");
  set_signal_handler(signal_handler);
  yield();
  log_event("hello again");
  set_signal_handler(other_signal_handler);
  yield();
  log_event("removing handler");
  set_signal_handler(NULL);
  yield();

  // TODO: does this work with self yield?
}

void setup(void) {
  set_thread_name(-1, "signaller");
  int tid = add_named_thread(worker, "receiver");

  // TODO: signal to an init thread is ignored
  thread_signal(tid);
  yield();

  // First handler
  thread_signal(tid);
  yield(); // runs handler
  yield(); // runs normal thread again

  // Second handler
  thread_signal(tid);
  yield();
  yield();

  // No handler
  thread_signal(tid);
  yield();
  yield();
}
