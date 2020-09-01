#ifndef COMMON_MUTEX_H
#define COMMON_MUTEX_H

#include <stddef.h>

typedef struct {
  size_t data;
} Mutex;

#define MUTEX_INIT   0
#define MUTEX_LOCK   1
#define MUTEX_UNLOCK 2

#endif /* ifdef COMMON_MUTEX_H */
