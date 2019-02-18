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
      if (all_threads[i]) {
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

__attribute__((noreturn)) void entry() {
  struct Thread temp_t1;
  init_thread(&temp_t1, temp_0, false);

  struct Thread temp_t2;
  init_thread(&temp_t2, temp_1, false);
  
  struct Thread counter_thread;
  init_thread(&counter_thread, counter, false);

  start_scheduler(); 
}
