#include <stddef.h>
#include "timer.h"

void enable_timer() {
  asm volatile ("svc 1":::"memory");
}

void disable_timer() {
  asm volatile ("svc 0":::"memory");
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
