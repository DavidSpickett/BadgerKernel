#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>

/* Define SYSCALL_ABI to enable the use of syscalls
   instead of calling functions directly. */
#define SYSCALL_ABI

typedef enum {
  syscall_add_named_thread = 0,
  syscall_add_thread,
  // Going to always include this to keep order intact
  syscall_add_thread_from_file,
  syscall_add_named_thread_with_args,
  syscall_eol,
} Syscall;

size_t generic_syscall(Syscall num, size_t arg1, size_t arg2, size_t arg3, size_t arg4);

// TODO: can we check size of __VA_ARG__ and pad somehow?
#ifdef SYSCALL_ABI
#define DO_SYSCALL_0(NAME) \
  generic_syscall(syscall_##NAME, 0, 0, 0, 0)
#define DO_SYSCALL_1(NAME, ARG1) \
  generic_syscall(syscall_##NAME, (size_t)ARG1, 0, 0, 0)
#define DO_SYSCALL_2(NAME, ARG1, ARG2) \
  generic_syscall(syscall_##NAME, (size_t)ARG1, (size_t)ARG2, 0, 0)
#define DO_SYSCALL_3(NAME, ARG1, ARG2, ARG3) \
  generic_syscall(syscall_##NAME, (size_t)ARG1, (size_t)ARG2, (size_t)ARG3, 0)
#define DO_SYSCALL_4(NAME, ARG1, ARG2, ARG3, ARG4) \
  generic_syscall(syscall_##NAME, (size_t)ARG1, (size_t)ARG2, (size_t)ARG3, (size_t)ARG4)
#else
#define DO_SYSCALL_0(NAME) \
  k_##NAME()
#define DO_SYSCALL_1(NAME, ARG1) \
  k_##NAME(ARG1)
#define DO_SYSCALL_2(NAME, ARG1, ARG2) \
  k_##NAME(ARG1, ARG2)
#define DO_SYSCALL_3(NAME, ARG1, ARG2, ARG3) \
  k_##NAME(ARG1, ARG2, ARG3)
#define DO_SYSCALL_4(NAME, ARG1, ARG2, ARG3, ARG4) \
  k_##NAME(ARG1, ARG2, ARG3, ARG4)
#endif

#endif /* ifdef SYSCALL_H */
