#include "common/assert.h"
#include "user/thread.h"

void first() {
  yield_to(2);
}

void second() {
  yield_to(1);
  yield_to(3); // switch to "last"
  yield();     // switch back to "last"
  // Run ourselves again, shouldn't actually do a switch
  assert(!yield());
  assert(!yield_to(1));
}

void setup(void) {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  add_named_thread(first, "first");
  add_named_thread(second, "second");
  add_named_thread(first, "last");
}
