#include "print.h"
#include "thread_state.h"
#include "util.h"
#include <stddef.h>

typedef enum {
syscall_foo = 0,
syscall_bar = 1,
syscall_cat = 2,
syscall_zzz = 3,
syscall_abc = 4,
syscall_eol,
} Syscall;

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
    "svc 21\n\t"
    :"+r"(r0)
    :"r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r7));
  return r0;
}

#define SYSCALL_ABI

// TODO: can we check size of __VA_ARG__ and pad somehow?
#ifdef SYSCALL_ABI
#define DO_SYSCALL_0(NAME) \
  generic_syscall(syscall_##NAME, 0, 0, 0, 0)
#define DO_SYSCALL_1(NAME, ARG1) \
  generic_syscall(syscall_##NAME, ARG1, 0, 0, 0)
#define DO_SYSCALL_2(NAME, ARG1, ARG2) \
  generic_syscall(syscall_##NAME, ARG1, ARG2, 0, 0)
#define DO_SYSCALL_3(NAME, ARG1, ARG2, ARG3) \
  generic_syscall(syscall_##NAME, ARG1, ARG2, ARG3, 0)
#define DO_SYSCALL_4(NAME, ARG1, ARG2, ARG3, ARG4) \
  generic_syscall(syscall_##NAME, ARG1, ARG2, ARG3, ARG4)
#else
#define DO_SYSCALL_0(NAME) \
  k_##NAME(ARG1)
#define DO_SYSCALL_1(NAME, ARG1) \
  k_##NAME(ARG1)
#define DO_SYSCALL_2(NAME, ARG1, ARG2) \
  k_##NAME(ARG1, ARG2)
#define DO_SYSCALL_3(NAME, ARG1, ARG2, ARG3) \
  k_##NAME(ARG1, ARG2, ARG3)
#define DO_SYSCALL_4(NAME, ARG1, ARG2, ARG3, ARG4) \
  k_##NAME(ARG1, ARG2, ARG3, ARG4)
#endif

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
