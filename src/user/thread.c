#include "user/thread.h"
#include "common/print.h"
#include "user/errno.h"
#include "user/syscall.h"
#include <stdarg.h>
#include <string.h>

// TODO: maybe logging should go via syscall
// to prevent splitting messages?
void log_event(const char* event, ...) {
  uint32_t k_config = get_kernel_config();
  if (!(k_config & KCFG_LOG_THREADS)) {
    return;
  }

  char formatted_name[THREAD_NAME_SIZE];
  char name[THREAD_NAME_SIZE];
  thread_name(CURRENT_THREAD, name);
  format_thread_name(formatted_name, get_thread_id(), name);
  printf("Thread %s: ", formatted_name);

  va_list args;
  va_start(args, event);
  vprintf(event, args);
  va_end(args);

  printf("\n");
}

int add_thread(const char* name, const ThreadArgs* args, void* worker,
               const ThreadFlags* flags) {
  ThreadArgs _args = {0, 0, 0, 0};
  if (args) {
    _args = *args;
  }
  return DO_SYSCALL_4(add_thread, name, &_args, worker, flags);
}

int add_named_thread(void (*worker)(void), const char* name) {
  ThreadFlags flags = {.is_file = false, .remove_permissions = 0};
  return add_thread(name, NULL, worker, &flags);
}

int add_thread_from_worker(void (*worker)(void)) {
  ThreadFlags flags = {.is_file = false, .remove_permissions = 0};
  return add_thread(NULL, NULL, worker, &flags);
}

int add_thread_from_file(const char* filename) {
  ThreadFlags flags = {.is_file = true, .remove_permissions = 0};
  return add_thread(filename, NULL, (void*)filename, &flags);
}

int add_thread_from_file_with_args(const char* filename,
                                   const ThreadArgs* args) {
  ThreadFlags flags = {.is_file = true, .remove_permissions = 0};
  return add_thread(filename, args, (void*)filename, &flags);
}

int add_named_thread_with_args(void (*worker)(), const char* name,
                               const ThreadArgs* args) {
  ThreadFlags flags = {.is_file = false, .remove_permissions = 0};
  return add_thread(name, args, worker, &flags);
}

void restart(void* worker, const char* name, const ThreadArgs* args,
             uint16_t remove_permissions) {
  DO_SYSCALL_4(restart, worker, name, args, remove_permissions);
}

int get_thread_id(void) {
  return user_thread_info.id;
}

bool thread_name(int tid, char* name) {
  if (tid == CURRENT_THREAD) {
    if (!name) {
      errno = E_INVALID_ARGS;
      return false;
    }

    memcpy(name, user_thread_info.name, THREAD_NAME_SIZE);
    return true;
  }
  return get_thread_property(tid, TPROP_NAME, name);
}

bool set_thread_name(int tid, const char* name) {
  return DO_SYSCALL_3(set_thread_property, tid, TPROP_NAME, name);
}

void set_kernel_config(uint32_t enable, uint32_t disable) {
  DO_SYSCALL_2(set_kernel_config, enable, disable);
}

uint32_t get_kernel_config(void) {
  return user_thread_info.kernel_config;
}

bool set_child(int child) {
  return DO_SYSCALL_3(set_thread_property, CURRENT_THREAD, TPROP_CHILD, &child);
}

bool get_thread_state(int tid, ThreadState* state) {
  bool got = get_thread_property(tid, TPROP_STATE, state);
  return got;
}

bool yield(void) {
  log_event("yielding");
  bool yielded = DO_SYSCALL_2(yield, INVALID_THREAD, YIELD_ANY);
  log_event("resuming");
  return yielded;
}

bool yield_to(int tid) {
  log_event("yielding");
  bool got = DO_SYSCALL_2(yield, tid, YIELD_TO);
  log_event("resuming");
  return got;
}

bool get_msg(int* sender, int* message) {
  return DO_SYSCALL_2(get_msg, sender, message);
}

bool send_msg(int destination, int message) {
  return DO_SYSCALL_2(send_msg, destination, message);
}

bool get_child(int tid, int* child) {
  bool got = get_thread_property(tid, TPROP_CHILD, child);
  return got;
}

bool get_parent(int tid, int* parent) {
  bool got = get_thread_property(tid, TPROP_PARENT, parent);
  return got;
}

uint16_t permissions(uint16_t remove) {
  set_thread_property(CURRENT_THREAD, TPROP_PERMISSIONS, &remove);
  uint16_t new_permissions = 0;
  get_thread_property(CURRENT_THREAD, TPROP_PERMISSIONS, &new_permissions);
  return new_permissions;
}

bool get_thread_registers(int tid, RegisterContext* regs) {
  return get_thread_property(tid, TPROP_REGISTERS, regs);
}

bool set_thread_registers(int tid, RegisterContext regs) {
  return set_thread_property(tid, TPROP_REGISTERS, &regs);
}

bool thread_signal(int tid, uint32_t signal) {
  return set_thread_property(tid, TPROP_PENDING_SIGNALS, &signal);
}

bool set_signal_handler(void (*handler)(uint32_t)) {
  return set_thread_property(CURRENT_THREAD, TPROP_SIGNAL_HANDLER, &handler);
}

bool get_thread_property(int tid, size_t property, void* res) {
  return DO_SYSCALL_3(get_thread_property, tid, property, res);
}

bool set_thread_property(int tid, size_t property, const void* value) {
  return DO_SYSCALL_3(set_thread_property, tid, property, value);
}

void thread_wait(void) {
  DO_SYSCALL_0(thread_wait);
}

bool thread_wake(int tid) {
  return DO_SYSCALL_1(thread_wake, tid);
}

bool thread_cancel(int tid) {
  return DO_SYSCALL_1(thread_cancel, tid);
}

bool thread_join(int tid, ThreadState* state) {
  // state may be NULL so use a local
  ThreadState got;
  while (1) {
    bool valid = get_thread_state(tid, &got);
    if (!valid) {
      return false;
    }

    if (got == finished || got == cancelled) {
      if (state) {
        *state = got;
      }
      return true;
    } else {
      yield();
    }
  }
}
