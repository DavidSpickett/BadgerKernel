#include "user/thread.h"
#include "thread.h"
#include "util.h"
#include <stddef.h>

void work() {
  int sender, message;
  get_msg(&sender, &message);

  while (message--) {
    yield();
  }

  log_event("Hello my name is %s", get_thread_name());
}

void spawner() {
  const char* demons[] = {
      "Morgoth",
      "Sauron",
      "Khamul",
  };
  const int dlen = 3;
  int ids[dlen];

  // Spawn new threads with the names above
  for (int idx = 0; idx < dlen; ++idx) {
    ids[idx] = add_named_thread(work, demons[idx]);
    // msg is number of turns to wait before printing
    send_msg(ids[idx], idx);
  }

  log_event("spawned demons");

  // Wait for them all to exit
  for (int j = 0; j < dlen; ++j) {
    thread_join(ids[j], NULL);
  }

  log_event("demons perished");
}

void setup(void) {
  KernelConfig cfg = { .log_scheduler=false,
                       .destroy_on_stack_err=false};
  k_set_kernel_config(&cfg);

  k_add_named_thread(spawner, "spawner");
}
