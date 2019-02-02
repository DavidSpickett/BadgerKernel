#include "thread.h"

__attribute__((noreturn)) void thread_worker_1() {
  while (1) {
    for (int i=0; i<100; ++i) {
      if ((i % 3) == 0) {
        log_event("working");
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

__attribute__((noreturn)) void main() {
  struct Thread thread1;
  init_thread(&thread1, thread_worker_0, false);
  struct Thread thread2;
  init_thread(&thread2, thread_worker_1, false);

  start_scheduler(); 
}
