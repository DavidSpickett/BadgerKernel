#include "print.h"
#include "user/thread.h"
#include "util.h"

void work(void) {
}

void go_to_sleep(void) {
  thread_wait();
}

void setup(void) {
  // Check we can escape %
  printf("%% Print Demo %%\n");
  printf("Signed numbers: %i %i %i %i %i\n", -2, -1, 0, 1, 2);

  char buf[100];
  const char* hex = "hex";
  sprintf(buf, "Sprintf %s: 0x%X\n", hex, 0xABAB);
  printf("%s", buf);

  printf("Padding 0x%8x 0x%08x 0x%10x\n",
    0xCDEF, 0xABCD, 0x3344);

  // Use up some IDs
  assert(add_thread_from_worker(go_to_sleep) != INVALID_THREAD);
  assert(add_thread_from_worker(go_to_sleep) != INVALID_THREAD);
  printf("Added 2 threads which are now sleeping.\n");

  // ID 2
  add_named_thread(work, "name_that_gets_cut_off");
  // The rest
  int tid = INVALID_THREAD;
  do {
    tid = add_thread_from_worker(work);
  } while (tid != INVALID_THREAD);
}
