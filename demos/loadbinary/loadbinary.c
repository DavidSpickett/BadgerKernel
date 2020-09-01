#include "common/assert.h"
#include "user/errno.h"
#include "user/thread.h"

const char* filename = "binary";
void load_again() {
  // Fails because other_worker from binary is still active
  // (worker has finished though)
  errno = 0;
  assert(add_thread_from_file(filename) == INVALID_THREAD);
  assert(errno == E_NO_PAGE);
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
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  assert(add_thread_from_file(filename) != INVALID_THREAD);
  add_named_thread(load_again, "load_again");
  // Fails because single code page is in use
  assert(add_thread_from_file(filename) == INVALID_THREAD);
}
