#include "common/syscall.h"
#include "common/print.h"
#include "kernel/thread.h"
#include <stddef.h>

static void k_invalid_syscall(size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t num) {
  printf("Unknown syscall %u invoked!\n", num);
  printf("arg1: %u, arg2: %u, arg3: %u, arg4: %u\n", arg1, arg2, arg3, arg4);
  k_exit(1);
}

typedef size_t (*SyscallFn)();
size_t k_handle_syscall(size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t num) {
  SyscallFn syscall_fn = NULL;

  switch (num) {
    case syscall_add_thread:
      syscall_fn = (SyscallFn)k_add_thread;
      break;
    case syscall_get_thread_property:
      syscall_fn = (SyscallFn)k_get_thread_property;
      break;
    case syscall_yield:
      syscall_fn = (SyscallFn)k_yield;
      break;
    case syscall_exit:
      syscall_fn = (SyscallFn)k_exit;
      break;
    case syscall_get_kernel_config:
      syscall_fn = (SyscallFn)k_get_kernel_config;
      break;
    case syscall_set_kernel_config:
      syscall_fn = (SyscallFn)k_set_kernel_config;
      break;
    case syscall_eol:
    default:
      k_invalid_syscall(arg1, arg2, arg3, arg4, num);
      break;
  }

  return syscall_fn(arg1, arg2, arg3, arg4);
}
