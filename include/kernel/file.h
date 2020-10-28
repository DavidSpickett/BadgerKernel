#ifndef KERNEL_FILE_H
#define KERNEL_FILE_H

#include "common/file.h"
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

int k_list_dir(const char* path, char* out, size_t outsz);
int k_open(const char* path, int oflag, ...);
ssize_t k_read(int filedes, void* buf, size_t nbyte);
ssize_t k_write(int filedes, const void* buf, size_t nbyte);
off_t k_lseek(int fd, off_t offset, int whence);
int k_remove(const char* path);
int k_close(int filedes);
int k_isatty(int fd);
bool k_stdout_isatty(void);

#endif /* ifdef KERNEL_FILE_H */
