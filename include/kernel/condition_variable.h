#ifndef KERNEL_CONDITION_VARIABLE_H
#define KERNEL_CONDITION_VARIABLE_H

#include "common/condition_variable.h"
#include <stddef.h>

bool k_condition_variable(unsigned op, ConditionVariable* vc);

#endif /* ifdef KERNEL_CONDITION_VARIABLE_H */
