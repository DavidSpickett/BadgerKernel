#include "thread.h"
#include "semihosting.h"

void temp_0() {
  for (int i=0; i<4; ++i) {
    yield();
  }
}

void temp_1() {
  for (int i=0; i<2; ++i) {
    yield();
  }
}

void counter() {
  int num_threads = 3;
  while (1) {
    int curr_threads = 0;
    for (size_t i=0; i<MAX_THREADS; ++i) { 
      if (is_valid_thread(i)) {
        ++curr_threads;
      }
    }
    
    if (curr_threads < num_threads) {
      log_event("a thread exited");
    }

    // not 0, because we'll still be running
    if (curr_threads == 1) {
      log_event("all threads exited");
      qemu_exit();
    }
    
    num_threads = curr_threads;
    yield();
  }
}

void demo() {
  add_thread(temp_0);
  add_thread(temp_1);
  add_thread(counter);

  start_scheduler(); 
}
