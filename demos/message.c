#include "thread.h"
#include "semihosting.h"

__attribute__((noreturn)) void sender() {
  while (1) {
    for (int i=0; ; ++i) {
      bool sent = false;
      while(!sent) {
        sent = send_msg(1, i);
        yield();
      }

      log_event("sent a message");
      yield();
    }
  }
}

__attribute__((noreturn)) void receiver() {
  while (1) {
      int sender, message;
      if (get_msg(&sender, &message)) {
        if (message == 10) {
          log_event("Got the right message");
          qemu_exit();
        }
        log_event("Got a different message");
      }
      yield();
  }
}

__attribute__((noreturn)) void entry() {
  struct Thread thread1;
  init_thread(&thread1, sender, false);
  struct Thread thread2;
  init_thread(&thread2, receiver, false);

  start_scheduler();
}
