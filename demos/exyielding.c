#include "thread.h"
#include "util.h"

void thread1() {
  yield_to(1);
}

void thread2() {
  yield_to(0);
  add_named_thread(thread1, "last");
  yield_next(); // switch to "last"
  yield_next(); // switch back to "last"
  // Run ourselves again, shouldn't actually do a switch
  if (yield_next()) {
    log_event("yield_next shouldn't have found another thread!");
  }
  ASSERT(!yield_to(0));
}

void setup(void) {
  add_named_thread(thread1, "first");
  add_named_thread(thread2, "second");
}
