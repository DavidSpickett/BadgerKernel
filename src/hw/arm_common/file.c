#include "kernel/file.h"
#include "common/assert.h"
#include "common/print.h"
#include "kernel/semihosting.h"
#include "kernel/thread.h"
#include <stddef.h>
#include <string.h>

int k_list_dir(const char* path, char* out, size_t outsz) {
  if (k_has_no_permission(TPERM_FILE)) {
    return -1;
  }

  // Semihosting doesn't provide an 'ls' command
  // so make our own with a temp file
  char cmd[128];
  const char* tmpname = "__ls.out";
  int len = sprintf(cmd, "ls %s > %s", path, tmpname);
  assert((len >= 0) && (len < 127));

  // Run the cmd
  size_t parameters[] = {(size_t)cmd, len};
  int ret = generic_semihosting_call(SYS_SYSTEM, parameters);
  if (ret != 0) {
    return ret;
  }

  int ls_fd = k_open(tmpname, O_RDONLY);
  assert(ls_fd != -1);
  parameters[0] = ls_fd;
  // Check output would fit in buffer
  int ls_sz = generic_semihosting_call(SYS_FLEN, parameters);
  assert(ls_sz < outsz);
  // Read whole file back
  k_read(ls_fd, out, ls_sz);
  k_close(ls_fd);

  return 0;
}

int k_open(const char* path, int oflag, ...) {
  if (k_has_no_permission(TPERM_FILE)) {
    return -1;
  }

  size_t parameters[] = {(size_t)path, oflag, strlen(path)};
  return generic_semihosting_call(SYS_OPEN, parameters);
}

ssize_t k_read(int fildes, void* buf, size_t nbyte) {
  if (k_has_no_permission(TPERM_FILE)) {
    return 0;
  }

  size_t parameters[] = {fildes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_READ, parameters);
  return nbyte - ret;
}

ssize_t k_write(int filedes, const void* buf, size_t nbyte) {
  if (k_has_no_permission(TPERM_FILE)) {
    return 0;
  }

  size_t parameters[] = {filedes, (size_t)buf, nbyte};
  size_t ret = generic_semihosting_call(SYS_WRITE, parameters);
  return nbyte - ret;
}

off_t k_lseek(int fd, off_t offset, int whence) {
  assert(whence == SEEK_CUR);
  if (k_has_no_permission(TPERM_FILE)) {
    return (off_t)-1;
  }

  size_t parameters[] = {fd, offset};
  int got = generic_semihosting_call(SYS_SEEK, parameters);
  return got == 0 ? offset : (off_t)-1;
}

int k_remove(const char* path) {
  if (k_has_no_permission(TPERM_FILE)) {
    return -1;
  }

  size_t parameters[] = {(size_t)path, strlen(path)};
  return generic_semihosting_call(SYS_REMOVE, parameters);
}

int k_close(int filedes) {
  if (k_has_no_permission(TPERM_FILE)) {
    return -1;
  }

  /* Need to make sure this is in initialised.
     If we use &filedes it thinks we only need
     the address of it. */
  size_t temp = filedes;
  return generic_semihosting_call(SYS_CLOSE, &temp);
}
