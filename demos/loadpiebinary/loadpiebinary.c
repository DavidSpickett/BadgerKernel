#include "user/thread.h"
#include "user/util.h"

// Dummy file to keep addLoadable happy
// File will be loaded via STARTUP_PROG

void setup(void) {
  log_event("Should have loaded piebinary via STARTUP_PROG!");
  exit(1);
}
