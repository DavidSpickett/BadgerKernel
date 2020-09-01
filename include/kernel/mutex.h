#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include <stdbool.h>
#include "common/mutex.h"

bool k_mutex(unsigned op, Mutex* m);

#endif /* ifdef KERNEL_MUTEX_H */
