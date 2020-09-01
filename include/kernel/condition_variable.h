#ifndef KERNEL_CONDITION_VARIABLE_H
#define KERNEL_CONDITION_VARIABLE_H

#include <stddef.h>
#include "common/condition_variable.h"

bool k_condition_variable(unsigned op, ConditionVariable* vc);

#endif /* ifdef KERNEL_CONDITION_VARIABLE_H */
