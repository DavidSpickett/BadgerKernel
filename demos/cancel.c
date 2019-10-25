#include "thread.h"
#include "util.h"

__attribute__((noreturn)) void work() {
  for (int num = 0; num < 3; ++num) {
    log_event("foo");
    yield();
  }
  __builtin_unreachable();
}

void canceller() {
  // Can't cancel an invalid thread
  bool did_cancel = thread_cancel(99);
  assert(!did_cancel);

  // So thread 2 is never run
  log_event("cancelling thread 2");
  thread_cancel(2);

  yield();

  log_event("cancelling thread 0");
  thread_cancel(0);

  yield();

  ThreadState state;
  // Can't join on an invalid thread
  bool finished = thread_join(99, &state);
  assert(!finished);

  thread_join(0, &state);
  assert(state == cancelled);
}

void setup(void) {
  config.log_scheduler = false;

  add_thread(work);
  add_thread(canceller);
  add_thread(work);
}
