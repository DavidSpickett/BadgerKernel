#include "util.h"
#include "semihosting.h"

void exit(int status) {
  unsigned event = get_semihosting_event(status);
  asm volatile (
      "ldr r0, =0x18\n\t" // SYS_EXIT
      "mov r1, %0\n\t"    // ADP_Stopped_ApplicationExit
      "svc 0xAB\n\t"      // semihosting call
      ::"r"(event)
      /* Otherwise the compiler might put "event" in r0
         and it'll get overwritten before use. */
      : "r0", "r1"
  );
}

// GDB helper to get current Cortex-M privilege level
// Yes, 1 means unprivileged. I know, weird right?
typedef enum { privileged, unprivileged } plevel;
plevel pl(void) {
  plevel level;
  asm volatile(
      "mrs r0, control\n\t"
      "mov r1, #1\n\t"
      "and r0, r0, r1\n\t"
      "mov %0, r0\n\t"
  : "=r"(level)
  );
  return level;
}
