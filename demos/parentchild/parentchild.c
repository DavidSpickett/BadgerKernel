#include "common/assert.h"
#include "user/thread.h"
#include "user/util.h"

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
  int parent = 0;
  assert(get_parent(tid, &parent));
  assert(parent == get_thread_id());

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
  // Thread 0 has no parent
  int parent = 0;
  assert(get_parent(get_thread_id(), &parent));
  assert(parent == INVALID_THREAD);

  // Can't get parent of an invalid thread
  assert(!get_parent(1, &parent));

  add_thread_from_worker(worker_1);
  add_thread_from_worker(worker_5);
}
