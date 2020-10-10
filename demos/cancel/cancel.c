#include "common/assert.h"
#include "user/thread.h"

__attribute__((noreturn)) void work() {
  for (int num = 0; num < 3; ++num) {
    log_event("foo");
    yield();
  }
  __builtin_unreachable();
}

void canceller() {
  // Can't cancel an invalid thread
  // Use MAX_THREADS here to cover some places
  // we use <= instead of < when validating it.
  bool did_cancel = thread_cancel(MAX_THREADS);
  assert(!did_cancel);

  // So thread 3 is never run
  log_event("cancelling thread 3");
  assert(thread_cancel(3));

  // Cancelled threads should be re-used
  int tid = add_thread_from_worker(work);
  assert(tid == 3);
  assert(thread_cancel(tid));

  yield();

  log_event("cancelling thread 1");
  thread_cancel(1);

  yield();

  ThreadState state;
  // Can't join on an invalid thread
  bool finished = thread_join(99, &state);
  assert(!finished);

  assert(thread_join(1, &state));
  assert(state == cancelled);

  assert(thread_cancel(0));
}

void cancel_self(void) {
  log_event("The Castle of Aarrgghh...");
  thread_cancel(CURRENT_THREAD);
  __builtin_unreachable();
}

void setup(void) {
  add_named_thread(cancel_self, "cancel_self");
  yield();

  add_thread_from_worker(work);
  add_thread_from_worker(canceller);
  add_thread_from_worker(work);

  // Keep thread 0 alive to prove that
  // Thread 3 is reused once cancelled
  thread_wait();
}
