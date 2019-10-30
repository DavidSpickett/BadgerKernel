#ifndef UTIL_H
#define UTIL_H

#include "print.h"
#include <stdbool.h>

/* Turns out, OCLint will complain about the macros
   in <assert.h> as well! Hooray. So we'll make our
   own macro, with solitaire and diet coke.
*/
__attribute__((noreturn))
void __assert_fail (const char *__assertion, const char *__file,
         unsigned int __line, const char *__function);

#ifdef NDEBUG
#define assert(expr) (void)expr
#else
#define assert(expr) \
{ \
  bool condition = expr; \
  condition ? 0 : __assert_fail(#expr, __FILE__, __LINE__, __func__); \
}
#endif

#ifdef linux
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#else
#include <stddef.h>
#include <sys/types.h>

// For semihosting assembly blocks
#ifdef __aarch64__
#define RCHR   "x"
#else
#define RCHR   "r"
#endif

// Semihosting operation codes
#define SYS_OPEN   0x01
#define SYS_CLOSE  0x02
#define SYS_WRITE  0x05
#define SYS_READ   0x06
#define SYS_REMOVE 0x0E
#define SYS_EXIT   0x18

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
