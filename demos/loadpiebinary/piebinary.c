#include "user/thread.h"
#include <limits.h>

const char* globalmsg = "Using a global!";

// This has a relocation on Thumb but not on
// Arm. As we only use its value rather than call it.
void foo() {
  log_event("I am a new thead!");
}

// This global function creates a relocation
// to this file's symbol table. Which has a value
// but must be offset by code_page to get the
// runtime address.
void disable_logging(void) {
  // Creates a .got section
  log_event("Disabling scheduler logging");
  set_kernel_config(0, KCFG_LOG_SCHEDULER);
}

// This does *not* need a relocation because
// its scope is the current file.
static void add_foo_thread(void) {
  log_event("Adding a new thread");
  add_named_thread(foo, "foo");
}

void worker() {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);
  yield();
  disable_logging();

  const char* msg = "Hello";
  // Creates a .plt section (and .plt.got I think)
  log_event(msg);

  add_foo_thread();
  yield_next();

  log_event("Goodbye");
}
