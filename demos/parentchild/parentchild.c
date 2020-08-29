#include "user/thread.h"
#include "user/util.h"
#include "util.h"

void worker_2() {
  log_event("Two!");
}
/* Three printed in worker_1 */
void worker_4() {
  log_event("Four!");
}
void worker_5() {
  log_event("Five!");
}

void worker_1() {
  log_event("One!");

  int tid = add_thread_from_worker(worker_2);
  assert(set_child(tid));

  // Won't run yet because we yield to worker_2
  // Then it comes back to us on finish
  int last_child_tid = add_thread_from_worker(worker_4);

  // Run the child thread
  // (not the second thread added in the setup function)
  yield();

  // Child returns here when it ends
  log_event("Three!");

  // Now we'll add a child thread but exit before it runs
  assert(set_child(last_child_tid));
  // We exit to the child here, which then chooses next
  // thread as normal since its parent has exited.
}

void setup(void) {
  add_thread_from_worker(worker_1);
  add_thread_from_worker(worker_5);
}
