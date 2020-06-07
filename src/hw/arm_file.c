#include "util.h"
#include "semihosting.h"
#include "file_system.h"
#include <stddef.h>
#include <string.h>

int k_open(const char* path, int oflag, ...) {
  volatile size_t parameters[] = {(size_t)path, oflag, strlen(path)};
  return generic_semihosting_call(SYS_OPEN, parameters);
}

ssize_t k_read(int fildes, void* buf, size_t nbyte) {
  volatile size_t parameters[] = {fildes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_READ, parameters);
  return nbyte - ret;
}

ssize_t k_write(int filedes, const void* buf, size_t nbyte) {
  volatile size_t parameters[] = {filedes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_WRITE, parameters);
  return nbyte - ret;
}

off_t k_lseek(int fd, off_t offset, int whence) { //!OCLINT
  assert(whence == SEEK_CUR);
  volatile size_t parameters[] = {fd, offset};
  int got = generic_semihosting_call(SYS_SEEK, parameters);
  return got == 0 ? offset : (off_t)-1;
}

int k_remove(const char* path) {
  volatile size_t parameters[] = {(size_t)path, strlen(path)};
  return generic_semihosting_call(SYS_REMOVE, parameters);
}

int k_close(int filedes) {
  /* Need to make sure this is in initialised.
     If we use &filedes it thinks we only need
     the address of it. */
  volatile size_t temp = filedes;
  return generic_semihosting_call(SYS_CLOSE, &temp);
}
