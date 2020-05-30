#include "thread.h"

const char* globalmsg = "Using a global!";

// This does not create a relocation because
// its location can de determined relative
// to the current pc.
// If we called it directly in this file it
// would have a relocation.
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
  config.log_scheduler = false;
}

// This does *not* need a relocation because
// its scope is the current file.
static void add_foo_thread(void) {
  log_event("Adding a new thread");
  add_named_thread(foo, "foo");
}

void worker() {
  disable_logging();

  const char* msg = "Hello";
  // Creates a .plt section (and .plt.got I think)
  log_event(msg);

  add_foo_thread();
  yield_next();

  log_event("Goodbye");
}
