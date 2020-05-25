#include "util.h"
#include <stddef.h>
#include <string.h>

/* Semihosting file handling */
extern size_t generic_semihosting_call(size_t operation,
                                       volatile size_t* parameters);

int open(const char* path, int oflag, ...) {
  volatile size_t parameters[] = {(size_t)path, oflag, strlen(path)};
  return generic_semihosting_call(SYS_OPEN, parameters);
}

ssize_t read(int fildes, void* buf, size_t nbyte) {
  volatile size_t parameters[] = {fildes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_READ, parameters);
  return nbyte - ret;
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
  volatile size_t parameters[] = {fildes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_WRITE, parameters);
  return nbyte - ret;
}

// Following the arm semihosting spec here not posix lseek
int seek(int filedes, size_t offset) {
  volatile size_t parameters[] = {filedes, offset};
  return generic_semihosting_call(SYS_SEEK, parameters);
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
