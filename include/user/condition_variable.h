#ifndef USER_CONDITION_VARIABLE_H
#define USER_CONDITION_VARIABLE_H

#include "common/condition_variable.h"
#include "common/macros.h"

BK_EXPORT void init_condition_variable(ConditionVariable* cv);
BK_EXPORT bool signal(ConditionVariable* cv);
BK_EXPORT void broadcast(ConditionVariable* cv);
BK_EXPORT void wait(ConditionVariable* cv);

#endif /* ifdef USER_CONDITION_VARIABLE_H */
