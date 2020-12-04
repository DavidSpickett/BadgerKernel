#include "common/assert.h"
#include "user/thread.h"
#include <stddef.h>

void work(int num) {
  for (int i = 0; i < num; ++i) {
    yield();
  }
}

void counter() {
  set_thread_name(CURRENT_THREAD, "counter");
  // Start at 1 so we don't join on ourselves
  for (int i = 1; i < 3; ++i) {
    ThreadState state = init;
    thread_join(i, &state);
    assert(state == finished);
    log_event("thread %i exited", i);
  }
}

void setup(void) {
  ThreadArgs ta1 = make_args(2, 0, 0, 0);
  add_named_thread_with_args(work, NULL, &ta1);

  ThreadArgs ta2 = make_args(4, 0, 0, 0);
  add_named_thread_with_args(work, NULL, &ta2);

  counter();
}
