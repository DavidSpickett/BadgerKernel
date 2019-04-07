#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

#include <stddef.h>
#include "thread.h"

typedef struct {
  int waiting[MAX_THREADS];
  size_t first;
  size_t last;
  bool full;
} ConditionVariable;

void init_condition_variable(ConditionVariable* cv);
bool signal(ConditionVariable* cv);
void broadcast(ConditionVariable* cv);
void wait(ConditionVariable* cv);

#endif /* ifdef CONDITION_VARIABLE_H */
