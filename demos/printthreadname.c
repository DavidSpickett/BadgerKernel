#include "print.h"
#include "thread.h"

void work(void) {
}

void setup(void) {
  config.log_scheduler = false;

  // Check we can escape %
  printf("%% Print Demo %%\n");

  // Use up some IDs
  unsigned padding = 8;
  for (int i = 0; i < padding; ++i) {
    add_thread(work);
    thread_cancel(i);
  }
  printf("Added then cancelled %u threads.\n", padding);

  // ID 7
  add_named_thread(work, "name_that_gets_cut_off");
  // The rest
  while (add_thread(work) != -1) { //!OCLINT
  }
}
