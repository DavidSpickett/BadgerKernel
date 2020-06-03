#include "print.h"
#include <stddef.h>

size_t handle_syscall(int arg1, int arg2, int arg3, int arg4) {
  printf("arg1: %u\n", arg1);
  printf("arg2: %u\n", arg2);
  printf("arg3: %u\n", arg3);
  printf("arg4: %u\n", arg4);
  return 1234 + arg1;
}

void sys_enable_timer(void) {
  printf("Enable timer!\n");
}

size_t generic_syscall(size_t num, size_t arg1, size_t arg2, size_t arg3, size_t arg4) {
  register unsigned r0 __asm("r0") = arg1;
  register unsigned r1 __asm("r1") = arg2;
  register unsigned r2 __asm("r2") = arg3;
  register unsigned r3 __asm("r3") = arg4;
  register unsigned r7 __asm("r7") = num;
  asm volatile(
    "svc 21\n\t"
    :"+r"(r0)
    :"r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r7));
  return r0;
}
