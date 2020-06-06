#include "user/thread.h"
#include "thread.h"
#include "util.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern void* _current_thread;

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
  size_t distance = (void*)(&dummy) - _current_thread;
  log_event("recursing");
  recurse(distance / ALLOC_SIZE);
  /* Don't log_event here because under some settings
     not all of the memory is 0. So name or id aren't 0. */
  yield();
}

void overflow() {
  char dummy;
  size_t distance = (void*)(&dummy) - _current_thread;
  log_event("overflowing");
  /* We're probably very close to top of stack
     so this is overkill. At least we can be sure that:
     (top of stack - sp) < (sp - current_thread)
     So overflow is garaunteed.
  */
  // Set -1 so scheduler doesn't check that position == ID
  memset((void*)&dummy, -1, distance);
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
  KernelConfig cfg = { .log_scheduler=false,
                       .destroy_on_stack_err=true};
  k_set_kernel_config(&cfg);

  k_add_named_thread(underflow, "underflow");
  k_add_named_thread(watcher, "watcher");
  k_add_named_thread(overflow, "overflow");
}
