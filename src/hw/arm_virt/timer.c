#include <stddef.h>
#include "timer.h"

void enable_timer() {
  asm volatile ("svc 1");
}

void disable_timer() {
  asm volatile ("svc 0");
}

/* GDB helpers */
size_t rtc() {
  size_t res;
  asm volatile ("mrc p15, 0, %0, c14, c2, 1"
    : "=r"(res)
  );
  return res;
}

size_t rt() {
  size_t res;
  // We're using the timer value not the count up value
  asm volatile ("mrc p15, 0, %0, c14, c2, 0"
    : "=r"(res)
  );
  return res;
}
