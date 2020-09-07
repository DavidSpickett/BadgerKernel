#include "kernel/generic_asm.h"
#include "kernel/semihosting.h"
#include "common/thread_state.h"
#include "common/print.h"
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

/* All parameters are declared voaltile to make
   sure that they are initialised even though
   we only use the address of them in these functions.
   generic_semihosting_call relies on their
   contents.
*/

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

/* Note: the file operations are in src/hw/arm_file.c
   so we can choose at build time whether to use those
   or the in memory file system. */

__attribute__((noreturn)) void __assert_fail(const char* __assertion,
                                             const char* __file,
                                             unsigned int __line,
                                             const char* __function) {
  printf("%s:%u (%s) Expected: %s\n", __file, __line, __function, __assertion);
  exit(1);
  __builtin_unreachable();
}
