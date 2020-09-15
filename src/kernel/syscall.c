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
  bool has_result = true;
  size_t result = 0;

  /* [[[cog
  import cog
  from scripts.syscalls import syscalls
  cog.outl("switch (num) {")
  for syscall, has_result in syscalls:
    cog.outl("  case syscall_{}:".format(syscall))
    cog.outl("    syscall_fn = (SyscallFn)k_{};".format(syscall))
    cog.outl("    has_result = {};".format("true" if has_result else "false"))
    cog.outl("    break;")
  ]]] */
  switch (num) {
    case syscall_add_thread:
      syscall_fn = (SyscallFn)k_add_thread;
      has_result = true;
      break;
    case syscall_get_thread_property:
      syscall_fn = (SyscallFn)k_get_thread_property;
      has_result = true;
      break;
    case syscall_set_thread_property:
      syscall_fn = (SyscallFn)k_set_thread_property;
      has_result = true;
      break;
    case syscall_get_kernel_config:
      syscall_fn = (SyscallFn)k_get_kernel_config;
      has_result = true;
      break;
    case syscall_set_kernel_config:
      syscall_fn = (SyscallFn)k_set_kernel_config;
      has_result = false;
      break;
    case syscall_yield:
      syscall_fn = (SyscallFn)k_yield;
      has_result = false;
      break;
    case syscall_get_msg:
      syscall_fn = (SyscallFn)k_get_msg;
      has_result = true;
      break;
    case syscall_send_msg:
      syscall_fn = (SyscallFn)k_send_msg;
      has_result = true;
      break;
    case syscall_thread_wait:
      syscall_fn = (SyscallFn)k_thread_wait;
      has_result = false;
      break;
    case syscall_thread_wake:
      syscall_fn = (SyscallFn)k_thread_wake;
      has_result = true;
      break;
    case syscall_thread_cancel:
      syscall_fn = (SyscallFn)k_thread_cancel;
      has_result = true;
      break;
    case syscall_mutex:
      syscall_fn = (SyscallFn)k_mutex;
      has_result = true;
      break;
    case syscall_condition_variable:
      syscall_fn = (SyscallFn)k_condition_variable;
      has_result = true;
      break;
    case syscall_open:
      syscall_fn = (SyscallFn)k_open;
      has_result = true;
      break;
    case syscall_read:
      syscall_fn = (SyscallFn)k_read;
      has_result = true;
      break;
    case syscall_write:
      syscall_fn = (SyscallFn)k_write;
      has_result = true;
      break;
    case syscall_lseek:
      syscall_fn = (SyscallFn)k_lseek;
      has_result = true;
      break;
    case syscall_remove:
      syscall_fn = (SyscallFn)k_remove;
      has_result = true;
      break;
    case syscall_close:
      syscall_fn = (SyscallFn)k_close;
      has_result = true;
      break;
    case syscall_exit:
      syscall_fn = (SyscallFn)k_exit;
      has_result = false;
      break;
    case syscall_malloc:
      syscall_fn = (SyscallFn)k_malloc;
      has_result = true;
      break;
    case syscall_realloc:
      syscall_fn = (SyscallFn)k_realloc;
      has_result = true;
      break;
    case syscall_free:
      syscall_fn = (SyscallFn)k_free;
      has_result = false;
      break;
    case syscall_list_dir:
      syscall_fn = (SyscallFn)k_list_dir;
      has_result = true;
      break;
    /* [[[end]]] */
    case syscall_eol:
    default:
      k_invalid_syscall(arg1, arg2, arg3, arg4, num);
      break;
  }

  result = syscall_fn(arg1, arg2, arg3, arg4);
  // Make sure we zero out the result if the syscall returned void
  // So we don't leak some in kernel address
  return has_result ? result : 0;
}
