#include "user/thread.h"
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
  assert(thread_cancel(2));

  // Cancelled threads should be re-used
  int tid = add_thread_from_worker(work);
  assert(tid == 2);
  assert(thread_cancel(tid));

  yield();

  log_event("cancelling thread 0");
  thread_cancel(0);

  yield();

  ThreadState state;
  // Can't join on an invalid thread
  bool finished = thread_join(99, &state);
  assert(!finished);

  assert(thread_join(0, &state));
  assert(state == cancelled);
}

void setup(void) {
  K_ADD_THREAD(work);
  K_ADD_THREAD(canceller);
  K_ADD_THREAD(work);
}
