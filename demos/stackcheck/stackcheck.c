#include "user/thread.h"
#include "thread.h"
#include "util.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern Thread* _current_thread;

#define ALLOC_SIZE 500
__attribute__((noinline)) void recurse(int repeat) {
  char dummy[ALLOC_SIZE];
  memset((void*)dummy, 0, ALLOC_SIZE);
  if (repeat) {
    recurse(repeat - 1);
  }
  return;
}

void underflow() {
  char dummy;
  size_t distance = (void*)(&dummy) - (void*)_current_thread;
  log_event("recursing");
  recurse(distance / ALLOC_SIZE);
  // Reset the name so we have consistent test output
  // To do this we need a correct thread ID
  _current_thread->id = 0;
  set_thread_name(CURRENT_THREAD, "underflowed");
  yield();
}

void overflow() {
  char dummy;
  size_t distance = (void*)(&dummy) - (void*)_current_thread;
  log_event("overflowing");
  /* We're probably very close to top of stack
     so this is overkill. At least we can be sure that:
     (top of stack - sp) < (sp - current_thread)
     So overflow is garaunteed.
  */
  // Set INVALID_THREAD so scheduler doesn't check that position == ID
  memset((void*)&dummy, INVALID_THREAD, distance);
  while (1) {
    yield();
  }
}

void watcher() {
  const int num_threads = 2;
  // Note we do not include ourselves
  int tids[] = {0, 2};

  for (int i = 0; i < num_threads; ++i) {
    if (thread_join(tids[i], NULL)) {
      log_event("thread did not error!");
    }
  }

  log_event("All threads errored");
}

void setup(void) {
  k_set_kernel_config(KCFG_DESTROY_ON_STACK_ERR,
    KCFG_LOG_SCHEDULER);

  K_ADD_NAMED_THREAD(underflow, "underflow");
  K_ADD_NAMED_THREAD(watcher, "watcher");
  K_ADD_NAMED_THREAD(overflow, "overflow");
}
