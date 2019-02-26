#include <stdint.h>
#include <stddef.h>
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
  bool threads_living = true;
  int our_id = get_thread_id();

  while (threads_living) {
    yield();
    threads_living = false;
    
    for (int id=0; id <= MAX_THREADS; ++id) {
      if ((id != our_id) &&
          is_valid_thread(id)) {
        threads_living = true;
        break;
      }
    }
  }

  log_event("All threads errored");
  qemu_exit();
}

void demo() {
  set_destroy_on_stack_err(true);

  add_named_thread(underflow, "underflow");
  add_named_thread(watcher, "watcher");
  add_named_thread(overflow, "overflow");

  start_scheduler(); 
}
