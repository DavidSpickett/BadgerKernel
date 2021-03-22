#include "kernel/semihosting.h"
#include "common/print.h"

// Arm semihosting routines
// platform specific asm in generic_semihosting_call

static size_t get_semihosting_event(int status) {
  if (status == 0) {
    return 0x20026; // ADP_Stopped_ApplicationExit
  }

  return 0x20024; // ADP_Stopped_InternalError
}

void k_exit(int status) {
  size_t event = get_semihosting_event(status);
#ifdef __aarch64__
  // Parameter pack on 64 bit
  size_t parameters[] = {event, 0 /* exit code */};
#else
  // Single argument for 32 bit
  size_t* parameters = (size_t*)event;
#endif
  generic_semihosting_call(SYS_EXIT, parameters);
#ifdef SEMIHOSTING_ENABLED
  __builtin_unreachable();
#else
  (void)status;
  printf("Exiting kernel\n");
  while (1) {
  }
#endif
}
