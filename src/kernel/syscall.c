#include "common/syscall.h"
#include "common/print.h"
#include "common/thread.h"
#include "kernel/alloc.h"
#include "kernel/condition_variable.h"
#include "kernel/file.h"
#include "kernel/message.h"
#include "kernel/mutex.h"
#include "kernel/thread.h"
#include "port/port.h"
#include <stddef.h>

static bool check_stack(void) {
  bool underflow = current_thread->top_canary != STACK_CANARY;
  bool overflow = current_thread->bottom_canary != STACK_CANARY;

  if (underflow || overflow) {
    /* Setting INVALID_THREAD here, instead of state=finished is fine,
       because A: the thread didn't actually finish
               B: the thread struct is actually invalid */
    current_thread->id = INVALID_THREAD;
    current_thread->name[0] = '\0';

    // Would clear heap allocs here but we can't trust the thread ID

    if (underflow) {
      k_log_event("Stack underflow!");
    }
    if (overflow) {
      k_log_event("Stack overflow!");
    }

    if (!(kernel_config & KCFG_DESTROY_ON_STACK_ERR)) {
      k_exit(1);
    }

    return false;
  }

  return true;
}

static const char* syscall_name(size_t num) {
  /* [[[cog
  import cog
  from scripts.syscalls import syscalls
  cog.outl("switch (num) {")
  for syscall, _, _ in syscalls:
    cog.outl("  case syscall_{}:".format(syscall))
    cog.outl("    return \"{}\";".format(syscall))
  ]]] */
  switch (num) {
    case syscall_add_thread:
      return "add_thread";
    case syscall_get_thread_property:
      return "get_thread_property";
    case syscall_set_thread_property:
      return "set_thread_property";
    case syscall_set_kernel_config:
      return "set_kernel_config";
    case syscall_yield:
      return "yield";
    case syscall_get_msg:
      return "get_msg";
    case syscall_send_msg:
      return "send_msg";
    case syscall_thread_wait:
      return "thread_wait";
    case syscall_thread_wake:
      return "thread_wake";
    case syscall_thread_cancel:
      return "thread_cancel";
    case syscall_mutex:
      return "mutex";
    case syscall_condition_variable:
      return "condition_variable";
    case syscall_open:
      return "open";
    case syscall_read:
      return "read";
    case syscall_write:
      return "write";
    case syscall_lseek:
      return "lseek";
    case syscall_remove:
      return "remove";
    case syscall_close:
      return "close";
    case syscall_exit:
      return "exit";
    case syscall_malloc:
      return "malloc";
    case syscall_realloc:
      return "realloc";
    case syscall_free:
      return "free";
    case syscall_list_dir:
      return "list_dir";
    /* [[[end]]] */
    case syscall_eol:
    default:
      return "(unknown syscall)";
  }
}

static void k_invalid_syscall(size_t arg1, size_t arg2, size_t arg3,
                              size_t arg4, size_t num) {
  printf("Unknown syscall %u invoked!\n", num);
  printf("arg1: %u, arg2: %u, arg3: %u, arg4: %u\n", arg1, arg2, arg3, arg4);
  k_exit(1);
}

void k_handle_syscall(void) {
  RegisterContext* ctx = (RegisterContext*)current_thread->stack_ptr;

  if (!check_stack()) {
    // Current thread now invalid, convert into a yield to pick
    // another valid thread. We will not return to the current.
    ctx->syscall_num = syscall_yield;
    ctx->arg1 = YIELD_ANY;
    ctx->arg0 = INVALID_THREAD;
  }

  void* syscall_fn = NULL;
  bool has_result = true;
  bool update_user_thread_info = false;
  size_t result = 0;

  /* [[[cog
  import cog
  from scripts.syscalls import syscalls
  cog.outl("switch (ctx->syscall_num) {")
  for syscall, has_result, update_user_thread_info in syscalls:
    cog.outl("  case syscall_{}:".format(syscall))
    cog.outl("    syscall_fn = k_{};".format(syscall))
    cog.outl("    has_result = {};".format("true" if has_result else "false"))
    cog.outl("    update_user_thread_info = {};".format(
      "true" if update_user_thread_info else "false"))
    cog.outl("    break;")
  ]]] */
  switch (ctx->syscall_num) {
    case syscall_add_thread:
      syscall_fn = k_add_thread;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_get_thread_property:
      syscall_fn = k_get_thread_property;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_set_thread_property:
      syscall_fn = k_set_thread_property;
      has_result = true;
      update_user_thread_info = true;
      break;
    case syscall_set_kernel_config:
      syscall_fn = k_set_kernel_config;
      has_result = false;
      update_user_thread_info = true;
      break;
    case syscall_yield:
      syscall_fn = k_yield;
      has_result = false;
      update_user_thread_info = false;
      break;
    case syscall_get_msg:
      syscall_fn = k_get_msg;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_send_msg:
      syscall_fn = k_send_msg;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_thread_wait:
      syscall_fn = k_thread_wait;
      has_result = false;
      update_user_thread_info = false;
      break;
    case syscall_thread_wake:
      syscall_fn = k_thread_wake;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_thread_cancel:
      syscall_fn = k_thread_cancel;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_mutex:
      syscall_fn = k_mutex;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_condition_variable:
      syscall_fn = k_condition_variable;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_open:
      syscall_fn = k_open;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_read:
      syscall_fn = k_read;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_write:
      syscall_fn = k_write;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_lseek:
      syscall_fn = k_lseek;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_remove:
      syscall_fn = k_remove;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_close:
      syscall_fn = k_close;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_exit:
      syscall_fn = k_exit;
      has_result = false;
      update_user_thread_info = false;
      break;
    case syscall_malloc:
      syscall_fn = k_malloc;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_realloc:
      syscall_fn = k_realloc;
      has_result = true;
      update_user_thread_info = false;
      break;
    case syscall_free:
      syscall_fn = k_free;
      has_result = false;
      update_user_thread_info = false;
      break;
    case syscall_list_dir:
      syscall_fn = k_list_dir;
      has_result = true;
      update_user_thread_info = false;
      break;
    /* [[[end]]] */
    case syscall_eol:
    default:
      k_invalid_syscall(ctx->arg0, ctx->arg1, ctx->arg2, ctx->arg3,
                        ctx->syscall_num);
      break;
  }

  bool log_errno = kernel_config & KCFG_LOG_FAILED_ERRNO;
  int old_err_no;
  if (log_errno) {
    // Callers aren't required to set errno to 0 so save current
    old_err_no = user_thread_info.err_no;
    user_thread_info.err_no = 0;
  }

  typedef size_t (*SyscallFn)();
  result =
      ((SyscallFn)(syscall_fn))(ctx->arg0, ctx->arg1, ctx->arg2, ctx->arg3);
  // Only copy to user if there's a result. To avoid leaking any kernel values.
  if (has_result) {
    ctx->arg0 = result;
  }

  if (log_errno) {
    if (user_thread_info.err_no != 0) {
      k_log_event("Syscall %s failed with errno %u (%s)!",
                  syscall_name(ctx->syscall_num), user_thread_info.err_no,
                  strerror(user_thread_info.err_no));
    } else {
      // The call didn't fail so restore the old errno
      // (even if that errno was a failure value, it's
      // not related to the current syscall)
      user_thread_info.err_no = old_err_no;
    }
  }

  // If we did something like yield, then go to the new thread
  // Otherwise return to the calling thread
  if (!next_thread) {
    next_thread = current_thread;
    if (update_user_thread_info) {
      k_update_user_thread_info(next_thread);
    }
  }
}
