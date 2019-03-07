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

    if (*your_flag) {
      *your_flag = false;
      *their_flag = true;
      *total_prints -= 1;

      if (*total_prints < 0) {
        break;
      }

      log_event(msg);
    }
    enable_timer();
    /* Note that for a sufficiently long timer amount,
       enable_timer and disable_timer are basically
       sequential.

       So to prevent a race condition we must check
       total_prints after our flag is set.
       Otherwise if the timer doesn't fire between
       enable and disable, we'll exit immediatley.
    */
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
