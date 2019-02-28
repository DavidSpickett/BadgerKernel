#include <string.h>
#include "timer.h"
#include "thread.h"
#include "semihosting.h"

void thread_work(const char* msg,
        bool* your_flag,
        bool* their_flag,
        int* total_prints
) {
  while (1) {
    disable_timer();

    // Check this *outside* the flag if
    // In case the other thread already exited
    if (*total_prints == 0) {
      break;
    }

    if (*your_flag) {
      log_event(msg);
      *your_flag = false;
      *their_flag = true;
      *total_prints -= 1;
    }
    enable_timer();
  }
  // Exit normally to scheduler...
}

void demo() {
  bool f1 = true;
  bool f2 = false;
  int num_prints = 4;

  struct ThreadArgs args1 = make_args("aardvark", &f1, &f2, &num_prints);
  add_named_thread_with_args(thread_work, NULL, args1);

  struct ThreadArgs args2 = make_args("zebra", &f2, &f1, &num_prints);
  add_named_thread_with_args(thread_work, NULL, args2);

  // Scheduler yields into first thread, timer does the rest
  start_scheduler();
}
