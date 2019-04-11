#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "thread.h"
#include "semihosting.h"

extern void* current_thread;

#define ALLOC_SIZE 500 
__attribute__((noinline))
void recurse(int n) {
  char f[ALLOC_SIZE];
  memset((void*)f, 0, ALLOC_SIZE);
  if (n) {
    recurse(n-1);
  }
  return;
}

void underflow() {
  char t;
  size_t distance = (void*)(&t)-current_thread;
  log_event("recursing");
  recurse(distance/ALLOC_SIZE);
  /* Don't log_event here because under some settings
     not all of the memory is 0. So name or id aren't 0. */
  yield();
}

void overflow() {
  char t;
  size_t distance = (void*)(&t)-current_thread;
  log_event("overflowing");
  /* We're probably very close to top of stack
     so this is overkill. At least we can be sure that:
     (top of stack - sp) < (sp - current_thread)
     So overflow is garaunteed.
  */
  // Set -1 so scheduler doesn't check that position == ID
  memset((void*)&t, -1, distance);
  while (1) { yield(); }
}

void watcher() {
  const int num_threads = 2;
  // Note we do not include ourselves
  int tids[] = {0, 2};

  for (int i=0; i<num_threads; ++i) {
    if (thread_join(tids[i], NULL)) {
      log_event("thread did not error!");
    }
  }

  log_event("All threads errored");
}

void demo() {
  config.log_scheduler = false;
  config.destroy_on_stack_err = true;

  add_named_thread(underflow, "underflow");
  add_named_thread(watcher, "watcher");
  add_named_thread(overflow, "overflow");

  start_scheduler(); 
}
