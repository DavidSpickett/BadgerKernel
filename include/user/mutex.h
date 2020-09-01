#ifndef USER_MUTEX_H
#define USER_MUTEX_H

#include <stdbool.h>
#include "common/mutex.h"

void init_mutex(Mutex* m);
bool unlock_mutex(Mutex* m);
bool lock_mutex(Mutex* m);

#endif /* ifdef USER_MUTEX_H */
