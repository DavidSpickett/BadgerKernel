#include <stddef.h>
#include "thread.h"
#include "semihosting.h"

void work() {
  int sender, message;
  get_msg(&sender, &message);

  while (message--) { yield(); }

  log_event("Hello my name is");
  log_event(get_thread_name());
}

__attribute__((noreturn)) void spawner() {
  const char* demons[] = {
    "Morgoth",
    "Sauron",
    "Khamul",
  };
  const int dlen = 3;
  int ids[dlen];

  // Spawn new threads with the names above
  for (int idx=0; idx < dlen; ++idx) {
    ids[idx] = add_named_thread(work, demons[idx]);
    // msg is number of turns to wait before printing
    send_msg(ids[idx], idx);
  }

  log_event("spawned demons");

  // Wait for them all to exit
  for (int j=0; j<dlen; ++j) {
    thread_join(ids[j], NULL);
  }

  log_event("demons perished");
  qemu_exit();
}

void setup(void) {
  config.log_scheduler = false;

  add_named_thread(spawner, "spawner");
}
