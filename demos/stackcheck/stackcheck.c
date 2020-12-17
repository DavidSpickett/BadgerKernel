// For current_thread
#include "kernel/thread.h"
#include "user/thread.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALLOC_SIZE 500
__attribute__((noinline)) void recurse(int repeat) {
  char dummy[ALLOC_SIZE];
  // Set all 1s so that our permissions will allow
  // us to reset the thread name later.
  memset((void*)dummy, -1, ALLOC_SIZE);
  if (repeat) {
    recurse(repeat - 1);
  }
  return;
}

void overflow() {
  set_thread_name(CURRENT_THREAD, "overflow");

  char dummy;
  size_t distance = (void*)(&dummy) - (void*)current_thread;
  log_event("recursing");
  recurse(distance / ALLOC_SIZE);
  // Reset the name so we have consistent test output
  // To do this we need a correct thread ID
  current_thread->id = 0;
  set_thread_name(CURRENT_THREAD, "overflowed");

  // Attempt to yield back to ourselves to show that
  // stack checks apply to yield_to. This will not return.
  yield_to(get_thread_id());
  __builtin_unreachable();
}

void underflow() {
  char dummy;
  size_t distance = (void*)(&dummy) - (void*)current_thread;
  log_event("underflowing");
  /* We're probably very close to top of stack
     so this is overkill. At least we can be sure that:
     (top of stack - sp) < (sp - current_thread)
     So underflow is garaunteed.
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
  set_kernel_config(KCFG_DESTROY_ON_STACK_ERR, KCFG_LOG_SCHEDULER);

  add_named_thread(watcher, "watcher");
  add_named_thread(underflow, "underflow");
  overflow();
}
