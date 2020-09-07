#include "common/thread_state.h"
#include "user/timer.h"
#include <stddef.h>

void enable_timer() {
  asm volatile("svc %0" : : "i"(svc_enable_timer) : "memory");
}

void disable_timer() {
  asm volatile("svc %0" : : "i"(svc_disable_timer) : "memory");
}

/* GDB helpers */

#ifdef __aarch64__
size_t rtc() {
  size_t res;
  asm volatile("mrs %0, CNTV_CTL_EL0" : "=r"(res));
  return res;
}

size_t rt() {
  size_t res;
  asm volatile("mrs %0, CNTVCT_EL0" : "=r"(res));
  return res;
}
#else
size_t rtc() {
  size_t res;
  asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r"(res));
  return res;
}

size_t rt() {
  size_t res;
  // We're using the timer value not the count up value
  asm volatile("mrc p15, 0, %0, c14, c2, 0" : "=r"(res));
  return res;
}
#endif // ifdef __aarch64__
