#include "thread.h"

void work(void) {}

void setup(void) {
  config.log_scheduler = false;

  // Use up some IDs
  for (int i=0; i<8; ++i) {
    add_thread(work);
    thread_cancel(i);
  }

  // ID 7
  add_thread(work);
  // Would be ID 9
  add_named_thread(work, "mushroom");
  // ID 10
  add_thread(work);
}
