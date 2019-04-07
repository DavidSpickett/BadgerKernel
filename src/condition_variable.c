#include "condition_variable.h"

void init_condition_variable(ConditionVariable* cv) {
  cv->first = 0;
  cv->last = 0;
  cv->full = false;
}

bool signal(ConditionVariable* cv) {
  if (cv->first != cv->last || cv->full) {
      thread_wake(cv->waiting[cv->first]);
      cv->first = (cv->first+1) % MAX_THREADS;
      cv->full = false;
      return true;
  }
  return false;
}

void broadcast(ConditionVariable* cv) {
  while (signal(cv)) {}
}

void wait(ConditionVariable *cv) {
  cv->waiting[cv->last] = get_thread_id();
  cv->last = (cv->last+1) % MAX_THREADS;
  cv->full = cv->first == cv->last;
  thread_wait();
}
