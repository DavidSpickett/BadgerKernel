#include "kernel/mutex.h"
#include "kernel/thread.h"

static void k_init_mutex(Mutex* m) {
  *m = INVALID_THREAD;
}

static bool k_unlock_mutex(Mutex* m) {
  if (k_get_thread_id() == *m) {
    *m = INVALID_THREAD;
    return true;
  }
  return false;
}

static bool k_lock_mutex(Mutex* m) {
  int id = k_get_thread_id();
  if (*m == INVALID_THREAD) {
    *m = id;
    return true;
  }
  return false;
}

bool k_mutex(unsigned op, Mutex* m) {
  switch (op) {
    case MUTEX_INIT:
      k_init_mutex(m);
      return true;
    case MUTEX_LOCK:
      return k_lock_mutex(m);
    case MUTEX_UNLOCK:
      return k_unlock_mutex(m);
    default:
      // TODO: E_INVALID_ARGS
      return false;
  }
}
