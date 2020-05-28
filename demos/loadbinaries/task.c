#include "thread.h"
#include "print.h"

void keep_page() {
  // Keep going back to load_again and trying find a free page
  // Which won't succeed until all of these have exited
  yield_to(1);
}

void worker() {
  int tid = get_thread_id();

  char buf[64];
  sprintf(buf, "from thread %u", tid);

  while (1) {
    // Some static to prove we copy code *and* data correctly
    static bool hello = true;
    if (hello) {
      log_event("Hello %s", buf);
      hello = false;
      yield();
    } else {
      log_event("Goodbye %s", buf);
      break;
    }
  }

  // These threads help us check that the lifetime
  // of the backing page is correct. As in, it can't
  // be re-used while these are live.
  log_event("Adding keepalive thread");
  add_thread(keep_page);
}
