#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

// These are semihosting values, not posix
#define O_RDONLY 0
#define O_WRONLY 6

// Only needed for Linux to make a new file
#define O_CREAT  0
// For Linux to make new file readable
#define S_IRUSR  0
// Seek to offset from beginning
#define SEEK_CUR 0

int k_open(const char* path, int oflag, ...);
ssize_t k_read(int filedes, void* buf, size_t nbyte);
ssize_t k_write(int filedes, const void* buf, size_t nbyte);
off_t k_lseek(int fd, off_t offset, int whence);
int k_remove(const char* path);
int k_close(int filedes);
void k_exit(int status);

#endif /* ifdef FILE_SYSTEM_H */
