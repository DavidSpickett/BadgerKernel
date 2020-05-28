#include "thread.h"
#include "util.h"

void load_again() {
  // This yielding is also testing that going from
  // non backed to backed and vice versa works
  int tid = -1;
  while (tid == -1) {
    yield();
    tid = add_thread_from_file("task2");
  }
  log_event("loaded again");
  // Check that skipping the scheduler still copies
  // in the new program. If not we'll see "Hello" instead
  // (also covers yield_next)
  yield_to(7);
}

void setup(void) {
  const char* filename = "task";

  add_thread_from_file(filename);
  // This will be able to load it again because page 0 was freed
  // when the first thread's keepalive exited
  add_named_thread(load_again, "load_again");

  add_thread_from_file(filename);
  add_thread_from_file(filename);
  // This fails because we've used up all the backing pages
  assert(add_thread_from_file(filename) == -1);
}
