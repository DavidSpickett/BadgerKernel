#include "user/thread.h"
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

void setup(void) {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  add_thread_from_worker(thread_worker_1);

  while (1) {
    log_event("working");
    yield();
  }
}
