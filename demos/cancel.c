#include "thread.h"
#include "util.h"

void work() {
  for (int num = 0; num < 3; ++num) {
    log_event("foo");
    yield();
  }
}

void canceller() {
  // Can't cancel an invalid thread
  ASSERT(!thread_cancel(99));

  // So thread 2 is never run
  log_event("cancelling thread 2");
  thread_cancel(2);

  yield();

  log_event("cancelling thread 0");
  thread_cancel(0);

  yield();

  ThreadState state;
  // Can't join on an invalid thread
  ASSERT(!thread_join(99, &state));

  thread_join(0, &state);
  ASSERT(state == cancelled);
}

void setup(void) {
  config.log_scheduler = false;

  add_thread(work);
  add_thread(canceller);
  add_thread(work);
}
