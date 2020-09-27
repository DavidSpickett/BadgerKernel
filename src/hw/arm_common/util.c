#include "common/generic_asm.h"
#include "common/print.h"
#include "common/thread_state.h"
#include "kernel/semihosting.h"
// For assert's exit
#include "user/util.h"
#include <stddef.h>
#include <string.h>

// Arm semihosting routines
// platform specific asm in generic_semihosting_call

static size_t get_semihosting_event(int status) {
  if (status == 0) {
    return 0x20026; // ADP_Stopped_ApplicationExit
  }

  return 0x20024; // ADP_Stopped_InternalError
}

size_t generic_semihosting_call(size_t operation, size_t* parameters) {
  size_t ret;
  // clang-format off
  // We assume that we're already in kernel mode by this point
  asm volatile (
    "mov "RCHR"0, %[operation]\n\t"
    "mov "RCHR"1, %[parameters]\n\t"
    SEMIHOSTING_CALL"\n\t"
    "mov %[ret], "RCHR"0\n\t"
    :[ret]"=r"(ret)
    :[parameters]"r"(parameters),
     [operation]"r"(operation)
    :RCHR"0", RCHR"1", "memory"
  );
  // clang-format on

  return ret;
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
  __builtin_unreachable();
}
