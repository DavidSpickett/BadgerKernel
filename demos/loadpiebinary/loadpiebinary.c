#include "thread.h"
#include "file_system.h"

// Dummy file to keep addLoadable happy
// File will be loaded via STARTUP_PROG

void setup(void) {
  k_log_event("Should have loaded piebinary via STARTUP_PROG!");
  k_exit(1);
}
