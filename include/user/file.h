#ifndef USER_FILE_H
#define USER_FILE_H

#include "common/file.h"
#include <stddef.h>
#include <sys/types.h>

int list_dir(const char* path, char* out, size_t outsz);
int open(const char* path, int oflag, ...);
ssize_t read(int filedes, void* buf, size_t nbyte);
ssize_t write(int filedes, const void* buf, size_t nbyte);
off_t lseek(int fd, off_t offset, int whence);
int remove(const char* path);
int close(int filedes);

#endif /* ifdef USER_FILE_H */
