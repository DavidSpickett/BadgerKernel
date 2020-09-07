#include "user/thread.h"
#include "user/util.h"

void signal_handler(uint32_t signal) {
  log_event("got signal %u", signal);
}

void other_signal_handler(uint32_t signal) {
  log_event("got a signal again, it was %u", signal);
}

void worker() {
  log_event("changing handler...");
  set_signal_handler(other_signal_handler);
  yield();
  log_event("removing handler");
  set_signal_handler(NULL);
  yield();
}

void setup(void) {
  set_thread_name(CURRENT_THREAD, "signaller");
  int tid = add_named_thread(worker, "receiver");

  // You'd probably never do this but just to prove
  // that an init thread can handle a signal even
  // before it's been run once.
  const void* handler_fn = signal_handler;
  set_thread_property(tid, TPROP_SIGNAL_HANDLER, &handler_fn);

  // This shows that even an init thread can handle a signal
  thread_signal(tid, 1);
  // Handles signal 1
  yield();

  // Actually runs, changes handler
  yield();

  // Stacked signals are handled on each yield until done
  thread_signal(tid, 2);
  thread_signal(tid, 3);
  // A repeated signal is ignored
  thread_signal(tid, 3);

  yield(); // Handle 2
  yield(); // Handle 3

  // Removes the handler
  yield();

  // All signals ignored when there's no handler
  thread_signal(tid, 4);
  thread_signal(tid, 5);
}
