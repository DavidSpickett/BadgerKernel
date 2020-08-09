#include "user/thread.h"
#include "syscall.h"
#include "print.h"
#include <stdarg.h>
#include <string.h>

#define THREAD_NAME_SIZE 12

void format_thread_name(char* out) {
  // fill with spaces (no +1 as we'll terminate it later)
  for (size_t idx = 0; idx < THREAD_NAME_SIZE; ++idx) {
    out[idx] = ' ';
  }

  const char* name;
  thread_name(-1, &name);

  if (name == NULL) {
    int tid = get_thread_id();

    // If the thread had a stack issue
    if (tid == -1) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_SIZE - h_len;
      strncpy(&out[padding], hidden, h_len);
    } else {
      // Just show the ID number (assume max 999 threads)
      char idstr[4];
      int len = sprintf(idstr, "%u", tid);
      strcpy(&out[THREAD_NAME_SIZE - len], idstr);
    }
  } else {
    size_t name_len = strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_SIZE) {
      name_len = THREAD_NAME_SIZE;
    }

    size_t padding = THREAD_NAME_SIZE - name_len;
    strncpy(&out[padding], name, name_len);
  }

  out[THREAD_NAME_SIZE] = '\0';
}

// TODO: maybe logging should go via syscall
// to prevent splitting messages?
void log_event(const char* event, ...) {
  if (!(get_kernel_config() & KCFG_LOG_THREADS)) {
    return;
  }

  char thread_name[THREAD_NAME_SIZE + 1];
  format_thread_name(thread_name);
  printf("Thread %s: ", thread_name);

  va_list args;
  va_start(args, event);
  vprintf(event, args);
  va_end(args);

  printf("\n");
}

int add_thread(const char* name, const ThreadArgs* args,
               void* worker, size_t kind) {
  return DO_SYSCALL_4(add_thread, name, args, worker, kind);
}

int add_named_thread(void (*worker)(void), const char* name) {
  return add_thread(name, NULL, worker, THREAD_FUNC);
}

int add_thread_from_worker(void (*worker)(void)) {
  return add_thread(NULL, NULL, worker, THREAD_FUNC);
}

int add_thread_from_file(const char* filename) {
  return add_thread(filename, NULL, (void*)filename, THREAD_FILE);
}

int add_thread_from_file_with_args(const char* filename, const ThreadArgs* args) {
  return add_thread(filename, args, (void*)filename, THREAD_FILE);
}

int add_named_thread_with_args(
      void (*worker)(), const char* name, const ThreadArgs *args) {
  return add_thread(name, args, worker, THREAD_FUNC);
}

int get_thread_id(void) {
  int ret;
  DO_SYSCALL_3(get_thread_property, -1,
    TPROP_ID, &ret);
  return ret;
}

bool thread_name(int tid, const char** name) {
  return DO_SYSCALL_3(get_thread_property, tid,
    TPROP_NAME, name);
}

void set_kernel_config(uint32_t enable, uint32_t disable) {
  DO_SYSCALL_2(set_kernel_config, enable, disable);
}

uint32_t get_kernel_config(void) {
  return DO_SYSCALL_0(get_kernel_config);
}

bool set_child(int child) {
  return DO_SYSCALL_1(set_child, child);
}

bool get_thread_state(int tid, ThreadState* state) {
  return DO_SYSCALL_3(get_thread_property, tid,
    TPROP_STATE, state);
}

void yield(void) {
  log_event("yielding");
  DO_SYSCALL_1(thread_yield, NULL);
  log_event("resuming");
}

bool yield_to(int tid) {
  log_event("yielding");
  bool got = DO_SYSCALL_1(yield_to, tid);
  log_event("resuming");
  return got;
}

bool yield_next(void) {
  log_event("yielding");
  bool got = DO_SYSCALL_0(yield_next);
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
  return DO_SYSCALL_3(get_thread_property, tid,
    TPROP_CHILD, child);
}

bool get_thread_property(int tid, size_t property,
                         size_t* res) {
  return DO_SYSCALL_3(get_thread_property,
                      tid, property, res);
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
