#include "common/assert.h"
#include "user/errno.h"
#include "user/thread.h"

// To check .bss loading
int zero_init_global = 0;

void load_again() {
  // Threads added by a loaded file share the code page
  // with that file, therefore the globals as well.
  assert(zero_init_global == 1);
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
  yield_to(tid);
}

void setup(void) {
  assert(zero_init_global == 0);
  zero_init_global = 1;

  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  const char* filename = "task";

  add_thread_from_file(filename);
  // This will be able to load it again because page 0 was freed
  // when the first thread's keepalive exited
  add_named_thread(load_again, "load_again");

  add_thread_from_file(filename);
  add_thread_from_file(filename);
  // This fails because we've used up all the backing pages
  errno = 0;
  assert(add_thread_from_file(filename) == INVALID_THREAD);
  assert(errno == E_NO_PAGE);
}
