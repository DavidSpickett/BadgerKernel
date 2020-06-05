#include "mutex.h"
#include "thread.h"

/* Surprise! The data is just the thread ID.
   At least it's hidden a little bit. */
#define ID(x)   (int)(x)
#define DATA(x) (size_t)(x)

void init_mutex(Mutex* mut) {
  mut->data = DATA(-1);
}

bool unlock_mutex(Mutex* m) {
  // TODO: does this take place in kernel or not?
  int id = k_get_thread_id();

  if (id == ID(m->data)) {
    m->data = DATA(-1);
    return true;
  }

  return false;
}

bool lock_mutex(Mutex* m) {
  // TODO: kernel or user?
  int id = k_get_thread_id();

  if (m->data == DATA(-1)) {
    m->data = DATA(id);
    return true;
  }

  return false;
}
