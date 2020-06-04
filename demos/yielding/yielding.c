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

extern int zzz();
extern int abc(int arg1);
extern int foo(int arg1, int arg2);
extern int bar(int arg1, int arg2, int arg3);
extern int cat(int arg1, int arg2, int arg3, int arg4);
__attribute__((noreturn)) void thread_worker_0() {
  log_event("zzz: %u", zzz());
  log_event("abc: %u", abc(8));
  log_event("foo: %u", foo(9, 8));
  log_event("bar: %u", bar(1, 2, 3));
  log_event("cat: %u", cat(5, 7, 8, 1));
  while(1) {}
}

void setup(void) {
  add_thread(thread_worker_0);
//  add_thread(thread_worker_1);
}
