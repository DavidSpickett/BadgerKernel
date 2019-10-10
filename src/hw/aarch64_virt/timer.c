#include <stddef.h>
#include "timer.h"
#include "thread_state.h"

void enable_timer() {
  asm volatile ("svc %0" :
    :"i"(svc_enable_timer)
    :"memory");
}

void disable_timer() {
  asm volatile ("svc %0" :
    :"i"(svc_disable_timer)
    :"memory");
}

// GDB helpers
size_t rtc() {
  size_t res;
  asm volatile ("mrs %0, CNTV_CTL_EL0"
    : "=r"(res)
  );
  return res;
}

size_t rt() {
  size_t res;
  asm volatile ("mrs %0, CNTVCT_EL0"
    : "=r"(res)
  );
  return res;
}
