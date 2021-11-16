#ifndef USER_FILE_H
#define USER_FILE_H

#include "common/file.h"
#include "common/macros.h"
#include <stddef.h>
#include <sys/types.h>

BK_EXPORT int list_dir(const char* path, char* out, size_t outsz);
BK_EXPORT int open(const char* path, int oflag, ...);
BK_EXPORT ssize_t read(int filedes, void* buf, size_t nbyte);
BK_EXPORT ssize_t write(int filedes, const void* buf, size_t nbyte);
BK_EXPORT off_t lseek(int fd, off_t offset, int whence);
BK_EXPORT int remove(const char* path);
BK_EXPORT int close(int filedes);

#endif /* ifdef USER_FILE_H */
