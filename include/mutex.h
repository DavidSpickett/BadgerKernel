#ifndef MUTEX_H
#define MUTEX_H

#include <stdbool.h>
#include <stddef.h>

struct Mutex {
  size_t data;
};

void init_mutex(struct Mutex* m);
bool unlock_mutex(struct Mutex* m);
bool lock_mutex(struct Mutex* m);

#endif /* ifdef MUTEX_H */
