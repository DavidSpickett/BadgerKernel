#include "common/print.h"
#include "common/generic_asm.h"
#include "common/thread_state.h"
#include "user/syscall.h"

size_t generic_syscall(Syscall num, size_t arg1, size_t arg2, size_t arg3,
                       size_t arg4) {
  register size_t reg0 __asm(RCHR "0") = arg1;
  register size_t reg1 __asm(RCHR "1") = arg2;
  register size_t reg2 __asm(RCHR "2") = arg3;
  register size_t reg3 __asm(RCHR "3") = arg4;

  /* r8 is loaded separatley to allow us to inline this
     function safely. It is not a caller saved register
     and can be corrupted by function calls placed between
     setting r8 and using it in the svc, particularly
     when LTO is enabled.
     r8 is used as r7 is the frame pointer on Thumb.
  */
  asm volatile("mov " RCHR "" SYSCALL_REG ", %[num]\n\t"
               "svc %[svc_syscall]\n\t"
               : "=r"(reg0)
               : "r"(reg0), "r"(reg1), "r"(reg2), "r"(reg3),
                 [ svc_syscall ] "i"(svc_syscall), [ num ] "r"((size_t)num)
               /* Clobbers the callee saved reg for num.
                  Kernel saves the rest for us. */
               : RCHR "" SYSCALL_REG, "memory");
  return reg0;
}
