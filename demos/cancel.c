#include "thread.h"

void work() {
  for (int i=0; i<3; ++i) {
    log_event("foo");
    yield();
  }
}

void canceller() {
  // So thread 2 is never run
  log_event("cancelling thread 2");
  thread_cancel(2);

  yield();

  log_event("cancelling thread 0");
  thread_cancel(0);

  yield();

  ThreadState ts;
  thread_join(0, &ts);
  if (ts != cancelled) {
    log_event("thread 0 not cancelled!");
  }
}

void demo() {
  config.log_scheduler = false;
  add_thread(work);
  add_thread(canceller);
  add_thread(work);

  start_scheduler();
}
