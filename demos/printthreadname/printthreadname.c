#include "common/assert.h"
#include "common/print.h"
#include "user/thread.h"

void work(void) {}

void go_to_sleep(void) {
  thread_wait();
}

#define PRINT_SPRINT(buf, fmt, ...)                                            \
  do {                                                                         \
    printf(fmt, ##__VA_ARGS__);                                                \
    sprintf(buf, fmt, ##__VA_ARGS__);                                          \
    printf("%s", buf);                                                         \
  } while (0)

void setup(void) {
  char buf[100];

  // Check we can escape %
  PRINT_SPRINT(buf, "%% printf/sprintf Demo %%\n");
  PRINT_SPRINT(buf, "Signed numbers: %i %i %i %i %i\n", -2, -1, 0, 1, 2);
  PRINT_SPRINT(buf, "Unsigned numbers: %u %u %u %u %u\n", 0, 1, 2, 3, 4);

  const char* hex = "hex";
  PRINT_SPRINT(buf, "print %s: 0x%X 0x%x\n", hex, 0xABAB, 0xBC);

  // Note that the first one doesn't need padding
  PRINT_SPRINT(buf, "Padding hex 0x%03x 0x%8x 0x%08X 0x%10x\n", 0x1234, 0xCDEF,
               0xABCD, 0x3344);
  // Same again, first won't be padded
  PRINT_SPRINT(buf, "Padding decimal %04i %2u %3i %10i\n", -23456, 3, -4,
               -12345);

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
