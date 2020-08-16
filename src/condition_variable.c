#include "condition_variable.h"
#include "user/thread.h"

void init_condition_variable(ConditionVariable* cond_var) {
  cond_var->first = 0;
  cond_var->last = 0;
  cond_var->full = false;
}

bool signal(ConditionVariable* cond_var) {
  if (cond_var->first != cond_var->last || cond_var->full) {
    thread_wake(cond_var->waiting[cond_var->first]);
    cond_var->first = (cond_var->first + 1) % MAX_THREADS;
    cond_var->full = false;
    return true;
  }
  return false;
}

void broadcast(ConditionVariable* cond_var) {
  bool signalled = false;
  do {
    signalled = signal(cond_var);
  } while (signalled);
}

void wait(ConditionVariable* cond_var) {
  // TODO: kernel or user?
  cond_var->waiting[cond_var->last] = k_get_thread_id();
  cond_var->last = (cond_var->last + 1) % MAX_THREADS;
  cond_var->full = cond_var->first == cond_var->last;
  // TODO: kernel or user?
  thread_wait();
}
