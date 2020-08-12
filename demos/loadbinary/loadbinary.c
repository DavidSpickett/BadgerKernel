#include "user/thread.h"
#include "thread.h"
#include "util.h"
#include <limits.h>

const char* filename = "binary";
void load_again() {
  // Fails because other_worker from binary is still active
  // (worker has finished though)
  assert(add_thread_from_file(filename) == INVALID_THREAD);
  yield();
  log_event("loading binary again");
  // Now that all threads from binary have finished
  // we can load it into the code page again
  int added = add_thread_from_file(filename);
  assert(added != INVALID_THREAD);
  // Cancelling it should release the page and let us load again
  assert(thread_cancel(added));
  added = add_thread_from_file(filename);
  assert(added != INVALID_THREAD);
}

void setup(void) {
  k_set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  assert(K_ADD_THREAD_FROM_FILE(filename) != INVALID_THREAD);
  K_ADD_NAMED_THREAD(load_again, "load_again");
  // Fails because single code page is in use
  assert(K_ADD_THREAD_FROM_FILE(filename) == INVALID_THREAD);
}
