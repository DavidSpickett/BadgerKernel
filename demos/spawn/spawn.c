#include "common/assert.h"
#include "user/thread.h"
#include <stddef.h>

void work() {
  int sender, message;
  assert(get_msg(&sender, &message));

  while (message--) {
    yield();
  }

  const char* tname;
  thread_name(CURRENT_THREAD, &tname);
  log_event("Hello my name is %s", tname);
}

void spawner() {
  set_thread_name(-1, "spawner");

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
  spawner();
}
