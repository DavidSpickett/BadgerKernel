#include "common/print.h"
#include "user/thread.h"

void keep_page(int tid) {
  // Keep going back to load_again and trying find a free page
  // Which won't succeed until all of these have exited
  log_event("keeping thread %u's page alive", tid);
  yield_to(2);
  log_event("releasing page for thread %u", tid);
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
  ThreadArgs args = make_args(get_thread_id(), 0, 0, 0);
  add_named_thread_with_args(keep_page, "keep_page", &args);
}
