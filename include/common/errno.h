#ifndef COMMON_ERRNO_H
#define COMMON_ERRNO_H

#define E_PERM         1 // Don't have permissions
#define E_INVALID_ID   2 // Invalid thread ID
#define E_NOT_FOUND    3 // File not found
#define E_NO_PAGE      4 // No code/backing page to load program into
#define E_INVALID_ARGS 5 // Incorrect arguments to syscall

char* strerror(int errnum);

#endif /* ifdef COMMON_ERRNO_H */
