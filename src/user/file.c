#include "user/file.h"
#include "user/syscall.h"
#include <stddef.h>

int list_dir(const char* path, char* out, size_t outsz) {
  return DO_SYSCALL_3(list_dir, path, out, outsz);
}

int open(const char* path, int oflag, ...) {
  // Ignoring the ...
  return DO_SYSCALL_2(open, path, oflag);
}

ssize_t read(int filedes, void* buf, size_t nbyte) {
  return DO_SYSCALL_3(read, filedes, buf, nbyte);
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
  return DO_SYSCALL_3(write, fildes, buf, nbyte);
}

off_t lseek(int fd, off_t offset, int whence) {
  return DO_SYSCALL_3(lseek, fd, offset, whence);
}

int remove(const char* path) {
  return DO_SYSCALL_1(remove, path);
}

int close(int filedes) {
  return DO_SYSCALL_1(close, filedes);
}
