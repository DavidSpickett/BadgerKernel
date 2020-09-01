#ifndef COMMON_CONDITION_VARIABLE_H
#define COMMON_CONDITION_VARIABLE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
  int waiting[MAX_THREADS];
  size_t first;
  size_t last;
  bool full;
} ConditionVariable;

#define CONDITION_VARIABLE_INIT      0
#define CONDITION_VARIABLE_SIGNAL    1
#define CONDITION_VARIABLE_BROADCAST 2
#define CONDITION_VARIABLE_WAIT      3

#endif /* ifdef COMMON_CONDITION_VARIABLE_H */
