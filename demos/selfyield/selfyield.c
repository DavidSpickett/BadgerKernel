#include "user/thread.h"
#include "thread.h"

/*
  Check that when switching threads via
  an interrupt (or something like it)
  we return to the exact place we left.
*/

void log_things() {
  log_event("one");

  // Manual thread switch (current=this thread, next=this thread)
  // Simulates an interrupt
  asm volatile("svc %0" : : "i"(svc_thread_switch) : "memory");

  log_event("two");
}

void thread(void) {
  log_things();
}

void setup(void) {
  k_set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  K_ADD_THREAD(thread);
}
