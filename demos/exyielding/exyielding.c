#include "common/assert.h"
#include "user/thread.h"

void thread1() {
  yield_to(2);
}

void thread2() {
  yield_to(1);
  add_named_thread(thread1, "last");
  yield(); // switch to "last"
  yield(); // switch back to "last"
  // Run ourselves again, shouldn't actually do a switch
  assert(!yield());
  assert(!yield_to(1));
}

void setup(void) {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  add_named_thread(thread1, "first");
  add_named_thread(thread2, "second");
}
