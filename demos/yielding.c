#include "thread.h"
#include "exit.h"

__attribute__((noreturn)) void thread_worker_1() {
  while (1) {
    for (int i=0; ; ++i) {
      if (i == 2) {
        log_event("working");
        log_event("exiting");
        qemu_exit();
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

__attribute__((noreturn)) void entry() {
  struct Thread thread1;
  init_thread(&thread1, thread_worker_0, false);
  struct Thread thread2;
  init_thread(&thread2, thread_worker_1, false);

  start_scheduler(); 
}
