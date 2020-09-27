#include "port/port.h"
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
