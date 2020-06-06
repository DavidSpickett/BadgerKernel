#include "print.h"
#include "thread_state.h"
#include "util.h"
#include "syscall.h"

size_t generic_syscall(Syscall num, size_t arg1, size_t arg2, size_t arg3, size_t arg4) {
  register size_t r0 __asm("r0") = arg1;
  register size_t r1 __asm("r1") = arg2;
  register size_t r2 __asm("r2") = arg3;
  register size_t r3 __asm("r3") = arg4;

  /* r7 is loaded seperatley to allow us to inline this
     function safely. It is not a caller saved register
     and can be corrupted by function calls placed between
     setting r7 and using it in the svc, particularly
     when LTO is enabled.
  */
  asm volatile(
    "mov r7, %[num]\n\t"
    "svc %[svc_syscall]\n\t"
    :"+r"(r0)
    :"r"(r0), "r"(r1), "r"(r2), "r"(r3),
     [svc_syscall]"i"(svc_syscall), [num]"r"(num)
  );
  return r0;
}
