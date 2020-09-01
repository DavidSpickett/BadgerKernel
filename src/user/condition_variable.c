#include "common/syscall.h"
#include "user/condition_variable.h"

void init_condition_variable(ConditionVariable* cv) {
  DO_SYSCALL_2(condition_variable, CONDITION_VARIABLE_INIT, cv);
}

bool signal(ConditionVariable* cv) {
  return DO_SYSCALL_2(condition_variable, CONDITION_VARIABLE_SIGNAL, cv);
}

void broadcast(ConditionVariable* cv) {
  DO_SYSCALL_2(condition_variable, CONDITION_VARIABLE_BROADCAST, cv);
}

void wait(ConditionVariable* cv) {
  DO_SYSCALL_2(condition_variable, CONDITION_VARIABLE_WAIT, cv);
}
