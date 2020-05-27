#include "thread.h"

const char* globalmsg = "Using a global!";

void other_worker() {
  log_event(globalmsg);
}

// The entry point
__attribute__((section(".worker")))
void worker() {
  const char* msg = "Hello";
  log_event(msg);
  add_named_thread(other_worker, "other");
  yield();
  log_event("Goodbye");
}
