#include "util.h"

void exit(int status) {
  unsigned event = get_semihosting_event(status);
  asm volatile (
      "ldr r0, =0x18\n\t"  // SYS_EXIT
      "mov r1, %0\n\t"     // ADP_Stopped_ApplicationExit
      "svc 0x00123456\n\t" // semihosting call
      ::"r"(event)
      /* Otherwise the compiler might put event in r0
         which will get overwritten before use. */
      : "r0", "r1"
  );
}
