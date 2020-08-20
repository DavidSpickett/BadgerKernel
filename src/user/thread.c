#include "user/thread.h"
#include "common/syscall.h"
#include "print.h"
#include <stdarg.h>
#include <string.h>

// TODO: maybe logging should go via syscall
// to prevent splitting messages?
void log_event(const char* event, ...) {
  if (!(get_kernel_config() & KCFG_LOG_THREADS)) {
    return;
  }

  char formatted_name[THREAD_NAME_SIZE + 1];
  const char* name;
  thread_name(CURRENT_THREAD, &name);
  format_thread_name(formatted_name, get_thread_id(),
                     name);
  printf("Thread %s: ", formatted_name);

  va_list args;
  va_start(args, event);
  vprintf(event, args);
  va_end(args);

  printf("\n");
}

int add_thread(const char* name, const ThreadArgs* args,
               void* worker, uint32_t flags) {
  // At some settings gcc decides (understandably) that
  // args is initialised but never read, so it doesn't bother.
  // This shows it that it will be read by the kernel and
  // saves adding volatile to add the interfaces.
  volatile ThreadArgs _args = {0,0,0,0};
  if (args) {
    _args = *args;
  }
  return DO_SYSCALL_4(add_thread, name, &_args, worker, flags);
}

int add_named_thread(void (*worker)(void), const char* name) {
  return add_thread(name, NULL, worker, THREAD_FUNC);
}

int add_thread_from_worker(void (*worker)(void)) {
  return add_thread(NULL, NULL, worker, THREAD_FUNC);
}

int add_thread_from_file(const char* filename) {
  return add_thread(filename, NULL, (void*)filename,
    THREAD_FILE);
}

int add_thread_from_file_with_args(const char* filename, const ThreadArgs* args) {
  return add_thread(filename, args, (void*)filename,
    THREAD_FILE);
}

int add_named_thread_with_args(
      void (*worker)(), const char* name, const ThreadArgs *args) {
  return add_thread(name, args, worker,
    THREAD_FUNC);
}

int get_thread_id(void) {
  // Volatile required to pick up result after syscall
  volatile int ret = 0;
  get_thread_property(-1, TPROP_ID, (size_t*)&ret);
  return ret;
}

bool thread_name(int tid, const char** name) {
  // Volatile to pick up syscall result
  const char* volatile _name = NULL;
  bool got = get_thread_property(tid,
    TPROP_NAME, (size_t*)&_name);
  *name = _name;
  return got;
}

bool set_thread_name(int tid, const char* name) {
  return DO_SYSCALL_3(set_thread_property, tid,
    TPROP_NAME, &name);
}

void set_kernel_config(uint32_t enable, uint32_t disable) {
  DO_SYSCALL_2(set_kernel_config, enable, disable);
}

uint32_t get_kernel_config(void) {
  return DO_SYSCALL_0(get_kernel_config);
}

bool set_child(int child) {
  return DO_SYSCALL_3(set_thread_property, -1,
    TPROP_CHILD, &child);
}

bool get_thread_state(int tid, ThreadState* state) {
  // Volatile to make sure we get the result of the syscall
  volatile ThreadState s = init;
  bool got = get_thread_property(tid,
    TPROP_STATE, (size_t*)&s);
  *state = s;
  return got;
}

void yield(void) {
  log_event("yielding");
  DO_SYSCALL_2(yield, INVALID_THREAD, YIELD_ANY);
  log_event("resuming");
}

bool yield_to(int tid) {
  log_event("yielding");
  bool got = DO_SYSCALL_2(yield, tid, YIELD_TO);
  log_event("resuming");
  return got;
}

bool yield_next(void) {
  log_event("yielding");
  bool got = DO_SYSCALL_2(yield, INVALID_THREAD, YIELD_NEXT);
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
  volatile int c = 0;
  bool got = get_thread_property(tid,
    TPROP_CHILD, (size_t*)&c);
  *child = c;
  return got;
}

uint16_t permissions(uint32_t remove) {
  uint16_t to_remove = remove >> TFLAG_PERM_SHIFT;
  set_thread_property(-1, TPROP_PERMISSIONS,
    &to_remove);
  volatile uint16_t new_permissions = 0;
  get_thread_property(-1, TPROP_PERMISSIONS,
    (size_t*)&new_permissions);
  return new_permissions;
}

bool get_thread_registers(int tid,
                  RegisterContext* regs) {
  volatile RegisterContext ret;
  bool got = get_thread_property(tid, TPROP_REGISTERS,
    (size_t*)&ret);
  *regs = ret;
  return got;
}

bool set_thread_registers(int tid, RegisterContext regs) {
  return set_thread_property(tid, TPROP_REGISTERS,
    &regs);
}

// Note that callers must apply volatile to their
// own res parameter. Since only they know the actual
// data type and therefore its correct size.
bool get_thread_property(int tid, size_t property,
                         void* res) {
  return DO_SYSCALL_3(get_thread_property,
                      tid, property, res);
}

bool set_thread_property(int tid, size_t property,
                         const void* value) {
  return DO_SYSCALL_3(set_thread_property,
                      tid, property, value);
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
