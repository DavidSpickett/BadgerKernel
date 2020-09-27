#include "port/port.h"
#include "common/arm_generic_asm.h"
#include "common/assert.h"

void do_svc(SVCCode code) {
  switch (code) {
    case svc_enable_timer:
      asm volatile("svc %0" : : "i"(svc_enable_timer) : "memory");
      break;
    case svc_disable_timer:
      asm volatile("svc %0" : : "i"(svc_disable_timer) : "memory");
      break;
    default:
      assert(0);
      __builtin_unreachable();
  }
}

size_t generic_semihosting_call(size_t operation, size_t* parameters) {
  size_t ret;
  // We assume that we're already in kernel mode by this point
  asm volatile("mov " RCHR "0, %[operation]\n\t"
               "mov " RCHR "1, %[parameters]\n\t"
               "" SEMIHOSTING_CALL "\n\t"
               "mov %[ret], " RCHR "0\n\t"
               : [ ret ] "=r"(ret)
               : [ parameters ] "r"(parameters), [ operation ] "r"(operation)
               : RCHR "0", RCHR "1", "memory");
  return ret;
}

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
