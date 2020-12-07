#include "kernel/condition_variable.h"
#include "common/errno.h"
#include "kernel/thread.h"

static bool k_init_condition_variable(ConditionVariable* cv) {
  cv->first = 0;
  cv->last = 0;
  cv->full = false;
  return true;
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

static bool k_broadcast(ConditionVariable* cv) {
  bool signalled = false;
  do {
    signalled = k_signal(cv);
  } while (signalled);
  return true;
}

static bool k_wait(ConditionVariable* cv) {
  cv->waiting[cv->last] = k_get_thread_id();
  cv->last = (cv->last + 1) % MAX_THREADS;
  cv->full = cv->first == cv->last;
  k_thread_wait();
  return true;
}

bool k_condition_variable(unsigned op, ConditionVariable* cv) {
  bool (*cv_func)(ConditionVariable*) = NULL;
  switch (op) {
    case CONDITION_VARIABLE_INIT:
      cv_func = k_init_condition_variable;
      break;
    case CONDITION_VARIABLE_SIGNAL:
      cv_func = k_signal;
      break;
    case CONDITION_VARIABLE_BROADCAST:
      cv_func = k_broadcast;
      break;
    case CONDITION_VARIABLE_WAIT:
      cv_func = k_wait;
      break;
  }

  if (!cv_func || !cv) {
    user_thread_info.err_no = E_INVALID_ARGS;
    return false;
  }

  return cv_func(cv);
}
