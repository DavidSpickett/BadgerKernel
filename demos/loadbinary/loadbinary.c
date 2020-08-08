#include "user/thread.h"
#include "thread.h"
#include "util.h"
#include <limits.h>

const char* filename = "binary";
void load_again() {
  // Fails because other_worker from binary is still active
  // (worker has finished though)
  assert(add_thread_from_file(filename) == -1);
  yield();
  log_event("loading binary again");
  // Now that all threads from binary have finished
  // we can load it into the code page again
  int added = add_thread_from_file(filename);
  assert(added != -1);
  // Cancelling it should release the page and let us load again
  assert(thread_cancel(added));
  added = add_thread_from_file(filename);
  assert(added != -1);
}

void setup(void) {
  k_set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  k_add_thread_from_file(filename);
  k_add_named_thread(load_again, "load_again");
  // Fails because single code page is in use
  assert(k_add_thread_from_file(filename) == -1);
}
