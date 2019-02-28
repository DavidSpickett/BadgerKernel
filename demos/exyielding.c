#include "thread.h"
#include "semihosting.h"

void thread1() {
  yield_to(1);
}

void thread2() {
  yield_to(0);
  add_named_thread(thread1, "last");
  yield_next(); // switch to "last"
  yield_next(); // switch back to "last"
  yield_next(); // run ourselves again
}

void demo() {
  add_named_thread(thread1, "first");
  add_named_thread(thread2, "second");

  start_scheduler(); 
}
