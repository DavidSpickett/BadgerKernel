#include "kernel/file.h"
#include "common/assert.h"
#include "common/print.h"
#include "kernel/semihosting.h"
#include "kernel/thread.h"
#include <stddef.h>
#include <string.h>

// Used to detect semihosting extensions
#define MAGIC_LEN    4
#define SHFB_MAGIC_0 0x53
#define SHFB_MAGIC_1 0x48
#define SHFB_MAGIC_2 0x46
#define SHFB_MAGIC_3 0x42

static bool k_host_supports_open_stdout(void) {
  int fd = k_open(":semihosting-features", O_RDONLY);
  if (fd == -1) {
    return false;
  }

  char magic[MAGIC_LEN];
  ssize_t got = k_read(fd, magic, MAGIC_LEN);
  if (got != MAGIC_LEN || magic[0] != SHFB_MAGIC_0 ||
      magic[1] != SHFB_MAGIC_1 || magic[2] != SHFB_MAGIC_2 ||
      magic[3] != SHFB_MAGIC_3) {
    k_close(fd);
    return false;
  }

  char feature_byte_1;
  got = k_read(fd, &feature_byte_1, 1);
  if (got != 1) {
    k_close(fd);
    return false;
  }

  k_close(fd);
  return feature_byte_1 & 2;
}

bool k_stdout_isatty(void) {
  if (!k_host_supports_open_stdout()) {
    return false;
  }

  int stdout_fd = k_open(":tt", O_WRONLY);
  if (stdout_fd == -1) {
    return false;
  }

  int istty = k_isatty(stdout_fd);
  k_close(stdout_fd);

  return istty == 1;
}

int k_isatty(int fd) {
  size_t param = fd;
  return generic_semihosting_call(SYS_ISTTY, &param);
}

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
  assert((ls_sz != -1) && (size_t)ls_sz < outsz);
  // Read whole file back
  k_read(ls_fd, out, ls_sz);
  // Null terminate
  out[ls_sz] = '\0';
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
