#include "user/thread.h"
#include "thread.h" // For setup's add thread
#include "user/util.h"

__attribute__((noreturn)) void thread_worker_1() {
  while (1) {
    for (int i = 0;; ++i) {
      if (i == 2) {
        log_event("working");
        log_event("exiting");
        exit(0);
      }
      yield();
    }
  }
}

__attribute__((noreturn)) void thread_worker_0() {
  while (1) {
    log_event("working");
    yield();
  }
}

void setup(void) {
  k_set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  K_ADD_THREAD(thread_worker_0);
  K_ADD_THREAD(thread_worker_1);
}
