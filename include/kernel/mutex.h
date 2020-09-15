#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include "common/mutex.h"
#include <stdbool.h>

bool k_mutex(unsigned op, Mutex* m);

#endif /* ifdef KERNEL_MUTEX_H */
