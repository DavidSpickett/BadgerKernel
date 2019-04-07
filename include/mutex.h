#ifndef MUTEX_H
#define MUTEX_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  size_t data;
} Mutex;

void init_mutex(Mutex* m);
bool unlock_mutex(Mutex* m);
bool lock_mutex(Mutex* m);

#endif /* ifdef MUTEX_H */
