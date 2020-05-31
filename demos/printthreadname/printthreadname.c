#include "print.h"
#include "thread.h"
#include "util.h"

void work(void) {
}

void go_to_sleep(void) {
  thread_wait();
}

void setup(void) {
  config.log_scheduler = false;

  // Check we can escape %
  printf("%% Print Demo %%\n");

  char buf[100];
  const char* hex = "hex";
  sprintf(buf, "Sprintf %s: 0x%X\n", hex, 0xABAB);
  printf("%s", buf);

  // Use up some IDs
  assert(add_thread(go_to_sleep) != -1);
  assert(add_thread(go_to_sleep) != -1);
  printf("Added 2 threads which are now sleeping.\n");

  // ID 2
  add_named_thread(work, "name_that_gets_cut_off");
  // The rest
  int tid = -1;
  do {
    tid = add_thread(work);
  } while (tid != -1);
}
