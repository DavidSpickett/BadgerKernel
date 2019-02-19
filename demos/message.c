#include "thread.h"
#include "semihosting.h"

__attribute__((noreturn)) void sender() {
  while (1) {
    if (send_msg(2, 99)) {
      log_event("sent a message");
    } else {
      log_event("message box was full");
    }

    yield();
  }
}

__attribute__((noreturn)) void spammer() {
  for (int i= 0; ; ++i) {
    if (i == 2) {
      // just one msg this time
      send_msg(2, -1);
      log_event("not spamming");
    } else {
      // Clog the receivers messages
      while (send_msg(2, -1)) {}
      log_event("spammed");
    }

    yield();
  }
}

__attribute__((noreturn)) void receiver() {
  while (1) {
    int sender, message;
    while (get_msg(&sender, &message)) {
      if (sender == 1) {
        log_event("got message from sender");
        qemu_exit();
      } else {
        log_event("discarded spam message");
      }
    }

    yield();
  }
}

void demo() {
  add_thread(spammer);
  add_thread(sender);
  add_thread(receiver);

  start_scheduler();
}
