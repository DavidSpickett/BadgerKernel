#include <string.h>
#include "timer.h"
#include "thread.h"
#include "semihosting.h"

const char* msgs[] = {
  "zero",
  "one",
  "two",
  NULL,
};
const char** curr_msg = msgs;

void thread_work() {
  if (!*curr_msg) {
    log_event("three");
    qemu_exit();
  }

  log_event(*curr_msg);
  curr_msg++;
  add_thread(thread_work);
  enable_timer();
  while (1); //!OCLINT
}

void demo() {
  add_thread(thread_work);
  start_scheduler();
}
