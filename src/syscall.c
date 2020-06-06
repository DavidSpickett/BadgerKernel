#include "print.h"
#include "thread_state.h"
#include "util.h"
#include "syscall.h"

// TODO: this breaks on LTO beyond O0
__attribute__((optimize("-O0")))
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
