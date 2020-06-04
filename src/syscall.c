#include "print.h"
#include "thread_state.h"
#include "util.h"
#include "syscall.h"
#include <stddef.h>

size_t k_zzz() {
  return 6666;
}

size_t k_abc(int arg1) {
  return 9999;
}

size_t k_foo(int arg1, int arg2) {
  return 1234;
}

size_t k_bar(int arg1, int arg2, int arg3) {
  return 5678;
}

size_t k_cat(int arg1, int arg2, int arg3, int arg4) {
  return 9876;
}

size_t generic_syscall(Syscall num, size_t arg1, size_t arg2, size_t arg3, size_t arg4) {
  register unsigned r0 __asm("r0") = arg1;
  register unsigned r1 __asm("r1") = arg2;
  register unsigned r2 __asm("r2") = arg3;
  register unsigned r3 __asm("r3") = arg4;
  register unsigned r7 __asm("r7") = num;

  assert(num < syscall_eol);

  asm volatile(
    "svc %[svc_syscall]\n\t"
    :"+r"(r0)
    :"r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r7),
     [svc_syscall]"i"(svc_syscall));
  return r0;
}

int zzz() {
  return DO_SYSCALL_0(zzz);
}

int abc(int arg1) {
  return DO_SYSCALL_1(abc, arg1);
}

int foo(int arg1, int arg2) {
  return DO_SYSCALL_2(foo, arg1, arg2);
}

int bar(int arg1, int arg2, int arg3) {
  return DO_SYSCALL_3(bar, arg1, arg2, arg3);
}

int cat(int arg1, int arg2, int arg3, int arg4) {
  return DO_SYSCALL_4(cat, arg1, arg2, arg3, arg4);
}
