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
#ifdef __thumb__
  asm volatile ("svc 0xff":::"memory");
#else
  asm volatile ("svc 0xdead":::"memory");
#endif

  log_event("two");
}

void thread(void) {
  log_things();
}

void setup(void) {
  add_thread(thread);
}
