#include "print.h"
#include "user/thread.h"
#include "thread.h"
#include "thread.h"
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
  assert(K_ADD_THREAD(go_to_sleep) != INVALID_THREAD);
  assert(K_ADD_THREAD(go_to_sleep) != INVALID_THREAD);
  printf("Added 2 threads which are now sleeping.\n");

  // ID 2
  K_ADD_NAMED_THREAD(work, "name_that_gets_cut_off");
  // The rest
  int tid = INVALID_THREAD;
  do {
    tid = K_ADD_THREAD(work);
  } while (tid != INVALID_THREAD);
}
