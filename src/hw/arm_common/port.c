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
  // clang-format off
  // We assume that we're already in kernel mode by this point
  asm volatile (
    "mov "RCHR"0, %[operation]\n\t"
    "mov "RCHR"1, %[parameters]\n\t"
    SEMIHOSTING_CALL"\n\t"
    "mov %[ret], "RCHR"0\n\t"
    :[ret]"=r"(ret)
    :[parameters]"r"(parameters),
     [operation]"r"(operation)
    :RCHR"0", RCHR"1", "memory"
  );
  // clang-format on

  return ret;
}
