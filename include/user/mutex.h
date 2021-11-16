#ifndef USER_MUTEX_H
#define USER_MUTEX_H

#include "common/macros.h"
#include "common/mutex.h"
#include <stdbool.h>

BK_EXPORT void init_mutex(Mutex* m);
BK_EXPORT bool unlock_mutex(Mutex* m);
BK_EXPORT bool lock_mutex(Mutex* m);

#endif /* ifdef USER_MUTEX_H */
