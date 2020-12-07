#include "kernel/mutex.h"
#include "common/errno.h"
#include "kernel/thread.h"

static bool k_init_mutex(Mutex* m) {
  *m = INVALID_THREAD;
  return true;
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
  bool (*mutex_fn)(Mutex*) = NULL;
  switch (op) {
    case MUTEX_INIT:
      mutex_fn = k_init_mutex;
      break;
    case MUTEX_LOCK:
      mutex_fn = k_lock_mutex;
      break;
    case MUTEX_UNLOCK:
      mutex_fn = k_unlock_mutex;
      break;
  }

  if (!mutex_fn || !m) {
    user_thread_info.err_no = E_INVALID_ARGS;
    return false;
  }

  return mutex_fn(m);
}
