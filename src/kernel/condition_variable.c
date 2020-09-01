#include "kernel/condition_variable.h"
#include "kernel/thread.h"

static void k_init_condition_variable(ConditionVariable* cv) {
  cv->first = 0;
  cv->last = 0;
  cv->full = false;
}

static bool k_signal(ConditionVariable* cv) {
  if (cv->first != cv->last || cv->full) {
    k_thread_wake(cv->waiting[cv->first]);
    cv->first = (cv->first + 1) % MAX_THREADS;
    cv->full = false;
    return true;
  }
  return false;
}

static void k_broadcast(ConditionVariable* cv) {
  bool signalled = false;
  do {
    signalled = k_signal(cv);
  } while (signalled);
}

static void k_wait(ConditionVariable* cv) {
  cv->waiting[cv->last] = k_get_thread_id();
  cv->last = (cv->last + 1) % MAX_THREADS;
  cv->full = cv->first == cv->last;
  k_thread_wait();
}

bool k_condition_variable(unsigned op, ConditionVariable* cv) {
  switch (op) {
    case CONDITION_VARIABLE_INIT:
      k_init_condition_variable(cv);
      return true;
    case CONDITION_VARIABLE_SIGNAL:
      return k_signal(cv);
    case CONDITION_VARIABLE_BROADCAST:
      k_broadcast(cv);
      return true;
    case CONDITION_VARIABLE_WAIT:
      k_wait(cv);
      return true;
    default:
      // TODO: E_INVALID_ARGS
      return false;
  }
}
