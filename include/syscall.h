#ifndef SYSCALL_H
#define SYSCALL_H

/* Define SYSCALL_ABI to enable the use of syscalls
   instead of calling functions directly. */
#define SYSCALL_ABI

typedef enum {
  syscall_foo = 0,
  syscall_bar = 1,
  syscall_cat = 2,
  syscall_zzz = 3,
  syscall_abc = 4,
  syscall_eol,
} Syscall;

// TODO: can we check size of __VA_ARG__ and pad somehow?
#ifdef SYSCALL_ABI
#define DO_SYSCALL_0(NAME) \
  generic_syscall(syscall_##NAME, 0, 0, 0, 0)
#define DO_SYSCALL_1(NAME, ARG1) \
  generic_syscall(syscall_##NAME, ARG1, 0, 0, 0)
#define DO_SYSCALL_2(NAME, ARG1, ARG2) \
  generic_syscall(syscall_##NAME, ARG1, ARG2, 0, 0)
#define DO_SYSCALL_3(NAME, ARG1, ARG2, ARG3) \
  generic_syscall(syscall_##NAME, ARG1, ARG2, ARG3, 0)
#define DO_SYSCALL_4(NAME, ARG1, ARG2, ARG3, ARG4) \
  generic_syscall(syscall_##NAME, ARG1, ARG2, ARG3, ARG4)
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
