#include "user/thread.h"
#include "syscall.h"

int add_named_thread(void (*worker)(void), const char* name) {
  return DO_SYSCALL_2(add_named_thread, worker, name);
}

int add_thread(void (*worker)(void)) {
  return DO_SYSCALL_1(add_thread, worker);
}

#if CODE_PAGE_SIZE
int add_thread_from_file(const char* filename) {
  return DO_SYSCALL_1(add_thread_from_file, filename);
}
#endif

// TODO: this will be broken as thread args is a copy !!!!!
int add_named_thread_with_args(
      void (*worker)(), const char* name, ThreadArgs args) {
  return DO_SYSCALL_3(add_named_thread_with_args, worker, name, args);
}

