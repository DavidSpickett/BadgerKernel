#ifndef SYSCALL_H
#define SYSCALL_H

// TODO: script generating this

#ifdef __ASSEMBLER__
#ifdef __aarch64__
#define FNADDR .quad
#else
#define FNADDR .word
#endif
  // Note: NO COMMAS ON THE ENDS!!
  FNADDR k_add_thread
  FNADDR k_get_thread_property
  FNADDR k_set_kernel_config
  FNADDR k_get_kernel_config
  FNADDR k_get_thread_state
  FNADDR k_set_child
  FNADDR k_get_child
  FNADDR k_thread_yield
  FNADDR k_yield_to
  FNADDR k_yield_next
  FNADDR k_get_msg
  FNADDR k_send_msg
  FNADDR k_open
  FNADDR k_read
  FNADDR k_write
  FNADDR k_lseek
  FNADDR k_remove
  FNADDR k_close
  FNADDR k_exit
  FNADDR k_malloc
  FNADDR k_realloc
  FNADDR k_free
  FNADDR k_list_dir
  FNADDR k_invalid_syscall
  FNADDR k_invalid_syscall
  FNADDR k_invalid_syscall
  FNADDR k_invalid_syscall
#else

#include <stddef.h>

typedef enum {
  syscall_add_thread = 0,
  syscall_get_thread_property,
  syscall_set_kernel_config,
  syscall_get_kernel_config,
  // TODO: sane ordering for these?
  syscall_get_thread_state,
  syscall_set_child,
  syscall_get_child,
  syscall_thread_yield,
  syscall_yield_to,
  syscall_yield_next,
  syscall_get_msg,
  syscall_send_msg,
  syscall_open,
  syscall_read,
  syscall_write,
  syscall_lseek,
  syscall_remove,
  syscall_close,
  syscall_exit,
  syscall_malloc,
  syscall_realloc,
  syscall_free,
  syscall_list_dir,
  syscall_eol,
} Syscall;

size_t generic_syscall(Syscall num, size_t arg1, size_t arg2, size_t arg3, size_t arg4);

// TODO: can we check size of __VA_ARG__ and pad somehow?
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

#endif /* ifdef __ASSEMBLER__ */
#endif /* ifdef SYSCALL_H */
