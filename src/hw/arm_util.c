#include "thread_state.h"
#include "util.h"
#include <stddef.h>
#include <string.h>

// Arm semihosting routines
// platform specific asm in generic_semihosting_call

size_t get_semihosting_event(int status) {
  if (status == 0) {
    return 0x20026; // ADP_Stopped_ApplicationExit
  }

  return 0x20024; // ADP_Stopped_InternalError
}

#ifdef __aarch64__
#define RCHR "x"
#define DIRECT_CALL "b .\n\t" // Unused
#elif defined __thumb__
#define RCHR "r"
#define DIRECT_CALL "bkpt 0xab\n\t"
#else /* Arm */
#define RCHR "r"
#define DIRECT_CALL "svc 0x123456\n\t"
#endif

size_t generic_semihosting_call(size_t operation, volatile size_t* parameters) {
  size_t ret;

  // clang-format off
  /* I never intend to run this function on Linux,
     but OCLint checks as if it were Linux and rejects
     this assembly. */
#ifdef linux
  ret = 0; (void)operation; (void)parameters;
#else
  /* Haven't figured out how to allow another exception
     if you're already handling the first one. So just
     see if we're already in the kernel first.
  */
#ifdef __thumb__
  size_t control;
  asm volatile ("mrs %0, control":"=r"(control));
  bool in_kernel = !(control & 0x2);
#elif defined __aarch64__
  bool in_kernel = false;
#else /* Arm */
  size_t cpsr;
  asm volatile ("mrs %0, cpsr":"=r"(cpsr));
  bool in_kernel = (cpsr & 0xF) == 3;
#endif /* defined __thumb__ */
  if (in_kernel) {
    asm volatile (
      "mov "RCHR"0, %[operation]\n\t"
      "mov "RCHR"1, %[parameters]\n\t"
      DIRECT_CALL
      "mov %[ret], "RCHR"0\n\t"
      :[ret]"=r"(ret)
      :[parameters]"r"(parameters),
       [operation]"r"(operation)
      :RCHR"0", RCHR"1"
    );
  } else {
    asm volatile (
      "mov "RCHR"0, %[operation]\n\t"
      "mov "RCHR"1, %[parameters]\n\t"
      "svc %[semihost]\n\t"
      "mov %[ret], "RCHR"0\n\t"
      :[ret]"=r"(ret)
      :[parameters]"r"(parameters),
       [operation]"r"(operation),
       [semihost]"i"(svc_semihosting)
      :RCHR"0", RCHR"1"
    );
  }
#endif /* ifdef linux */
  // clang-format on

  return ret;
}

/* All parameters are declared voaltile to make
   sure that they are initialised even though
   we only use the address of them in these functions.
   generic_semihosting_call relies on their
   contents.
*/

void exit(int status) {
  size_t event = get_semihosting_event(status);
#ifdef __aarch64__
  // Parameter pack on 64 bit
  volatile size_t parameters[] = {event, 0 /* exit code */};
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
