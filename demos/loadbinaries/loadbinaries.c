#include "user/thread.h"
#include "thread.h"
#include "util.h"

void load_again() {
  // This yielding is also testing that going from
  // non backed to backed and vice versa works
  int tid = INVALID_THREAD;
  while (tid == INVALID_THREAD) {
    yield();
    tid = add_thread_from_file("task2");
  }
  log_event("loaded again");

  // Cancelling this new thread should release its backing page
  assert(thread_cancel(tid));
  tid = add_thread_from_file("task2");
  assert(tid != INVALID_THREAD);

  // Check that skipping the scheduler still copies
  // in the new program. If not we'll see "Hello" instead
  // (also covers yield_next)
  yield_to(tid);
}

void setup(void) {
  k_set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  const char* filename = "task";

  K_ADD_THREAD_FROM_FILE(filename);
  // This will be able to load it again because page 0 was freed
  // when the first thread's keepalive exited
  K_ADD_NAMED_THREAD(load_again, "load_again");

  K_ADD_THREAD_FROM_FILE(filename);
  K_ADD_THREAD_FROM_FILE(filename);
  // This fails because we've used up all the backing pages
  assert(K_ADD_THREAD_FROM_FILE(filename) == INVALID_THREAD);
}
