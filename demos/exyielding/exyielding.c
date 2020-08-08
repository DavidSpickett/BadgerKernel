#include "thread.h"
#include "user/thread.h"
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
  bool yielded = yield_next();
  assert(!yielded);
  yielded = yield_to(0);
  assert(!yielded);
}

void setup(void) {
  k_set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  k_add_named_thread(thread1, "first");
  k_add_named_thread(thread2, "second");
}
