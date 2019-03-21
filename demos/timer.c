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
    if (*your_flag) {
      /* Disable the timer after we've found the flag
         to prevent a race condition. If we did:
         disable timer -> check flag -> enable timer
         The timer may not fire between enable/disable.
         Whether it does depends on code layout and machine speed.
         With this ordering we might be interrupted while
         reading our flag but that's fine. We'll just pick
         it up next time aronud the loop.
         (and there's no need for waiting for interrupts)
      */
      disable_timer();

      *your_flag = false;
      *their_flag = true;
      *total_prints -= 1;

      if (*total_prints < 0) {
        break;
      }

      log_event(msg);
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
