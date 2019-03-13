#include "thread.h"
#include "semihosting.h"

__thread int i = 2;
// Specifically an array to show that vars with size != 4 work
__thread char msg[7] = {'H', 'e', 'l', 'l', 'o', '!', '\0'};

void thread_worker() {
  // Make i different for each thread
  i = i * (get_thread_id() + 1);
  for ( ; i > 0; --i) {
      log_event(msg);
      yield();
  }
}

void demo() {
  add_thread(thread_worker);
  add_thread(thread_worker);

  start_scheduler();
}
