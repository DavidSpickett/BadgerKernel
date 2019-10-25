#ifndef UTIL_H
#define UTIL_H

#include "print.h"
#include <stdbool.h>

#ifdef linux
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#else
#include <stddef.h>
#include <sys/types.h>

__attribute__((noreturn))
void __assert_fail (const char *__assertion, const char *__file,
         unsigned int __line, const char *__function);

#ifdef NDEBUG
#define assert(expr) (void)expr
#else
#define assert(expr) \
expr ? 0 : __assert_fail(#expr, __FILE__, __LINE__, __func__)
#endif

// These are semihosting values, not posix
#define O_RDONLY 0
#define O_WRONLY 6

// Only needed for Linux to make a new file
#define O_CREAT  0
// For Linux to make new file readable
#define S_IRUSR  0

int open(const char* path, int oflag, ...);
ssize_t read(int fildes, void* buf, size_t nbyte);
ssize_t write(int fildes, const void* buf, size_t nbyte);
int remove(const char* path);
int close(int filedes);
void exit(int status);

#endif

#endif /* ifdef UTIL_H */
