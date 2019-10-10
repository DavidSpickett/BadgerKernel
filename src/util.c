#include "util.h"
#include <stddef.h>
#include <string.h>

// Arm semihosting routines
// platform specific asm in generic_semihosting_call

#define SYS_OPEN   0x01
#define SYS_CLOSE  0x02
#define SYS_WRITE  0x05
#define SYS_READ   0x06
#define SYS_REMOVE 0x0E
#define SYS_EXIT   0x18

size_t get_semihosting_event(int status) {
  if (status == 0) {
    return 0x20026; // ADP_Stopped_ApplicationExit
  }

  return 0x20024; // ADP_Stopped_InternalError
}

#ifdef __aarch64__
#define RCHR   "x"
#else
#define RCHR   "r"
#endif

static size_t generic_semihosting_call(size_t operation,
                                       volatile size_t* parameters) {
  size_t ret;

  // clang-format off
  asm volatile (
    "mov "RCHR"0, %[operation]\n\t"
    "mov "RCHR"1, %[parameters]\n\t"
    "svc 1\n\t"
    "mov %[ret], "RCHR"0\n\t"
    :[ret]"=r"(ret)
    :[parameters]"r"(parameters), [operation]"r"(operation)
    :RCHR"0", RCHR"1"
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

int open(const char* path, int oflag, ...) {
  volatile size_t parameters[] = {(size_t)path, oflag, strlen(path)};
  return generic_semihosting_call(SYS_OPEN, parameters);
}

size_t read(int fildes, void* buf, size_t nbyte) {
  volatile size_t parameters[] = {fildes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_READ, parameters);
  return nbyte - ret;
}

size_t write(int fildes, const void* buf, size_t nbyte) {
  volatile size_t parameters[] = {fildes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_WRITE, parameters);
  return nbyte - ret;
}

int remove(const char* path) {
  volatile size_t parameters[] = {(size_t)path, strlen(path)};
  return generic_semihosting_call(SYS_REMOVE, parameters);
}

int close(int filedes) {
  /* Need to make sure this is in initialised.
     If we use &filedes it thinks we only need
     the address of it. */
  volatile size_t temp = filedes;
  return generic_semihosting_call(SYS_CLOSE, &temp);
}

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
}
