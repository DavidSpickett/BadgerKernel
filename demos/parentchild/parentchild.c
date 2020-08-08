#include "user/thread.h"
#include "thread.h" // For setup's add thread
#include "user/util.h"
#include "util.h"

void worker_1() { log_event("One!"); }
/* Two printed in worker_0 */
void worker_3() { log_event("Three!"); }
void worker_4() { log_event("Four!"); }

void worker_0() {
  log_event("Zero!");

  int tid = add_thread(worker_1);
  assert(set_child(tid));

  // Won't run yet because we yield to worker_1
  // Then it comes back to us on finish
  int last_child_tid = add_thread(worker_3);

  // Run the child thread
  // (not the second thread added in the setup function)
  yield();

  // Child returns here when it ends
  log_event("Two!");

  // Now we'll add a child thread but exit before it runs
  assert(set_child(last_child_tid));
  // We exit to the child here, which then chooses next
  // thread as normal since its parent has exited.
}

void setup(void) {
  KernelConfig cfg = { .log_scheduler=false,
                       .log_threads=true,
                       .destroy_on_stack_err=false};
  k_set_kernel_config(&cfg);
  k_add_thread(worker_0);
  k_add_thread(worker_4);
}
