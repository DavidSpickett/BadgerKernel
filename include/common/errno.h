#ifndef COMMON_ERRNO_H
#define COMMON_ERRNO_H

#include "user/thread.h"

#define errno *__get_errno()

// Note: kernel should set via thread struct
int* __get_errno(void);

#define E_PERM       1
#define E_INVALID_ID 2
#define E_NOT_FOUND  3

#endif /* ifdef COMMON_ERRNO_H */
