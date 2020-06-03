#include "thread.h"
#include "util.h"

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

extern size_t generic_syscall(size_t num, size_t arg1, size_t arg2, size_t arg3, size_t arg4);
__attribute__((noreturn)) void thread_worker_0() {
  //while (1) {
  //  log_event("working");
  //  yield();
  //}
  log_event("Doing call");
  size_t res = generic_syscall(1, 9, 8, 7, 6);
  log_event("Got: %u", res);
  res = generic_syscall(0, 6, 5, 4, 3);
  log_event("Got: %u", res);
  while(1) {}
}

void setup(void) {
  add_thread(thread_worker_0);
//  add_thread(thread_worker_1);
}
