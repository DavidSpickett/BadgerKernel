#ifndef USER_CONDITION_VARIABLE_H
#define USER_CONDITION_VARIABLE_H

#include "common/condition_variable.h"

void init_condition_variable(ConditionVariable* cv);
bool signal(ConditionVariable* cv);
void broadcast(ConditionVariable* cv);
void wait(ConditionVariable* cv);

#endif /* ifdef USER_CONDITION_VARIABLE_H */
