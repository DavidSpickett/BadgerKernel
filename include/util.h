#ifndef UTIL_H
#define UTIL_H

#include "print.h"

#define str(x) #x
#define str_(x) str(x)
#define STR__LINE__ str_(__LINE__)
#define ASSERT(expr) \
  if (!(expr)) { /*!OCLINT*/ \
    printf("%s\n", __FILE__":"STR__LINE__" Expected: "#expr); \
    exit(1); \
  }

#ifdef linux
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <stddef.h>

// These are semihosting values, not posix
#define O_RDONLY 0
#define O_WRONLY 6

// Only needed for Linux to make a new file
#define O_CREAT 0
// For Linux to make new file readable
#define S_IRUSR 0

int open(const char* path, int oflag, ...);
size_t read(int fildes, void* buf, size_t nbyte);
size_t write(int fildes, const void *buf, size_t nbyte);
int remove(const char *path);
int close(int filedes);
void exit(int status);

#endif

#endif /* ifdef UTIL_H */
