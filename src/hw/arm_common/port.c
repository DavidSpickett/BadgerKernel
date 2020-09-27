#include "port/port.h"
#include "common/assert.h"

#ifdef __thumb__
extern void thumb_enable_timer(void);
extern void thumb_disable_timer(void);
#endif

void do_svc(SVCCode code) {
  switch (code) {
    case svc_enable_timer:
#ifdef __thumb__
      thumb_enable_timer();
#else
      asm volatile("svc %0" : : "i"(svc_enable_timer) : "memory");
#endif
      break;
    case svc_disable_timer:
#ifdef __thumb__
      thumb_disable_timer();
#else
      asm volatile("svc %0" : : "i"(svc_disable_timer) : "memory");
#endif
      break;
    default:
      assert(0);
      __builtin_unreachable();
  }
}
