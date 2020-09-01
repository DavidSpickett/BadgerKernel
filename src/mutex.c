#include "kernel/mutex.h"
#include "kernel/thread.h"

/* Surprise! The data is just the thread ID.
   At least it's hidden a little bit. */
#define ID(x)   (int)(x)
#define DATA(x) (size_t)(x)

bool k_mutex(unsigned op, Mutex * m) {
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

void k_init_mutex(Mutex* mut) {
  mut->data = DATA(-1);
}

bool k_unlock_mutex(Mutex* m) {
  // TODO: does this take place in kernel or not?
  int id = k_get_thread_id();

  if (id == ID(m->data)) {
    m->data = DATA(-1);
    return true;
  }

  return false;
}

bool k_lock_mutex(Mutex* m) {
  // TODO: kernel or user?
  int id = k_get_thread_id();

  if (m->data == DATA(-1)) {
    m->data = DATA(id);
    return true;
  }

  return false;
}
