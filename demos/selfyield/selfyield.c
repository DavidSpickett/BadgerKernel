#include "port/port.h"
#include "user/thread.h"

/*
  Check that when switching threads via
  an interrupt (or something like it)
  we return to the exact place we left.
*/

void log_things() {
  log_event("one");

  // Manual thread switch which simulates an interrupt
  YIELD_ASM;

  log_event("two");
}

void setup(void) {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);
  log_things();
}
