#include "print.h"

void handle_syscall(size_t num) {
  printf("Asked to handle call %u\n", num);
}

void sys_enable_timer(void) {
  printf("Enable timer!\n");
}
