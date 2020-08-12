#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include "file_common.h"

int k_list_dir(const char* path, char* out, size_t outsz);
int k_open(const char* path, int oflag, ...);
ssize_t k_read(int filedes, void* buf, size_t nbyte);
ssize_t k_write(int filedes, const void* buf, size_t nbyte);
off_t k_lseek(int fd, off_t offset, int whence);
int k_remove(const char* path);
int k_close(int filedes);

#endif /* ifdef FILE_H */
