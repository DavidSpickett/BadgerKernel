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
  KernelConfig cfg = { .log_scheduler=false,
                       .destroy_on_stack_err=false};
  k_set_kernel_config(&cfg);

  // Check we can escape %
  printf("%% Print Demo %%\n");

  char buf[100];
  const char* hex = "hex";
  sprintf(buf, "Sprintf %s: 0x%X\n", hex, 0xABAB);
  printf("%s", buf);

  // Use up some IDs
  assert(k_add_thread(go_to_sleep) != -1);
  assert(k_add_thread(go_to_sleep) != -1);
  printf("Added 2 threads which are now sleeping.\n");

  // ID 2
  k_add_named_thread(work, "name_that_gets_cut_off");
  // The rest
  int tid = -1;
  do {
    tid = k_add_thread(work);
  } while (tid != -1);
}
