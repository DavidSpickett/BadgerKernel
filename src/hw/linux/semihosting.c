#include <stdlib.h>
#include <stdio.h>

void qemu_exit(void) {
  exit(0);
}

void qemu_print(const char* msg) {
  printf("%s", msg);
}
