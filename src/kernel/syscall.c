#include "common/syscall.h"
#include "common/print.h"
#include "kernel/thread.h"
#include "kernel/mutex.h"
#include "kernel/alloc.h"
#include "kernel/file.h"
#include "kernel/condition_variable.h"
#include <stddef.h>

static void k_invalid_syscall(size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t num) {
  printf("Unknown syscall %u invoked!\n", num);
  printf("arg1: %u, arg2: %u, arg3: %u, arg4: %u\n", arg1, arg2, arg3, arg4);
  k_exit(1);
}

typedef size_t (*SyscallFn)();
size_t k_handle_syscall(size_t arg1, size_t arg2, size_t arg3, size_t arg4, size_t num) {
  SyscallFn syscall_fn = NULL;

  /* [[[cog
  import cog
  from scripts.syscalls import syscalls
  cog.outl("switch (num) {")
  for syscall in syscalls:
    cog.outl("  case syscall_{}:".format(syscall))
    cog.outl("    syscall_fn = (SyscallFn)k_{};".format(syscall))
    cog.outl("    break;")
  ]]] */
  switch (num) {
    case syscall_add_thread:
      syscall_fn = (SyscallFn)k_add_thread;
      break;
    case syscall_get_thread_property:
      syscall_fn = (SyscallFn)k_get_thread_property;
      break;
    case syscall_set_thread_property:
      syscall_fn = (SyscallFn)k_set_thread_property;
      break;
    case syscall_get_kernel_config:
      syscall_fn = (SyscallFn)k_get_kernel_config;
      break;
    case syscall_set_kernel_config:
      syscall_fn = (SyscallFn)k_set_kernel_config;
      break;
    case syscall_yield:
      syscall_fn = (SyscallFn)k_yield;
      break;
    case syscall_get_msg:
      syscall_fn = (SyscallFn)k_get_msg;
      break;
    case syscall_send_msg:
      syscall_fn = (SyscallFn)k_send_msg;
      break;
    case syscall_thread_wait:
      syscall_fn = (SyscallFn)k_thread_wait;
      break;
    case syscall_thread_wake:
      syscall_fn = (SyscallFn)k_thread_wake;
      break;
    case syscall_thread_cancel:
      syscall_fn = (SyscallFn)k_thread_cancel;
      break;
    case syscall_mutex:
      syscall_fn = (SyscallFn)k_mutex;
      break;
    case syscall_condition_variable:
      syscall_fn = (SyscallFn)k_condition_variable;
      break;
    case syscall_open:
      syscall_fn = (SyscallFn)k_open;
      break;
    case syscall_read:
      syscall_fn = (SyscallFn)k_read;
      break;
    case syscall_write:
      syscall_fn = (SyscallFn)k_write;
      break;
    case syscall_lseek:
      syscall_fn = (SyscallFn)k_lseek;
      break;
    case syscall_remove:
      syscall_fn = (SyscallFn)k_remove;
      break;
    case syscall_close:
      syscall_fn = (SyscallFn)k_close;
      break;
    case syscall_exit:
      syscall_fn = (SyscallFn)k_exit;
      break;
    case syscall_malloc:
      syscall_fn = (SyscallFn)k_malloc;
      break;
    case syscall_realloc:
      syscall_fn = (SyscallFn)k_realloc;
      break;
    case syscall_free:
      syscall_fn = (SyscallFn)k_free;
      break;
    case syscall_list_dir:
      syscall_fn = (SyscallFn)k_list_dir;
      break;
    /* [[[end]]] */
    case syscall_eol:
    default:
      k_invalid_syscall(arg1, arg2, arg3, arg4, num);
      break;
  }

  return syscall_fn(arg1, arg2, arg3, arg4);
}
