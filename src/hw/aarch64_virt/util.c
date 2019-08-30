#include <stdint.h>
#include "util.h"
#include "semihosting.h"

void exit(int status) {
  unsigned event = get_semihosting_event(status);
  // Exit code unused here since Arm/Thumb don't have the option
  uint64_t ret[] = {event, 0 /*exit code*/};
  asm volatile (
    "mov x1, %0\n\t"           // address of parameter block
    "mov w0, #0x18\n\t"        // SYS_EXIT
    "svc 0x3333\n\t"           // ask monitor to do semihosting call
    ::"r"(ret)
    :"x1", "w0", "memory" // To make sure ret gets initialised
  );
}
