#ifndef USER_MUTEX_H
#define USER_MUTEX_H

#include "common/mutex.h"
#include <stdbool.h>

void init_mutex(Mutex* m);
bool unlock_mutex(Mutex* m);
bool lock_mutex(Mutex* m);

#endif /* ifdef USER_MUTEX_H */
