#include "user/mutex.h"
#include "user/syscall.h"

void init_mutex(Mutex* m) {
  DO_SYSCALL_2(mutex, MUTEX_INIT, m);
}

bool lock_mutex(Mutex* m) {
  return DO_SYSCALL_2(mutex, MUTEX_LOCK, m);
}

bool unlock_mutex(Mutex* m) {
  return DO_SYSCALL_2(mutex, MUTEX_UNLOCK, m);
}
