#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#include "common/syscall.h"
#include <stddef.h>

size_t generic_syscall(Syscall num, size_t arg1, size_t arg2,
                       size_t arg3, size_t arg4);

#define DO_SYSCALL_0(NAME) \
  generic_syscall(syscall_##NAME, 0, 0, 0, 0)
#define DO_SYSCALL_1(NAME, ARG1) \
  generic_syscall(syscall_##NAME, (size_t)(ARG1), 0, 0, 0)
#define DO_SYSCALL_2(NAME, ARG1, ARG2) \
  generic_syscall(syscall_##NAME, (size_t)(ARG1), (size_t)(ARG2), 0, 0)
#define DO_SYSCALL_3(NAME, ARG1, ARG2, ARG3) \
  generic_syscall(syscall_##NAME, (size_t)(ARG1), (size_t)(ARG2), (size_t)(ARG3), 0)
#define DO_SYSCALL_4(NAME, ARG1, ARG2, ARG3, ARG4) \
  generic_syscall(syscall_##NAME, (size_t)(ARG1), (size_t)(ARG2), (size_t)(ARG3), (size_t)(ARG4))

#endif /* ifdef USER_SYSCALL_H */
