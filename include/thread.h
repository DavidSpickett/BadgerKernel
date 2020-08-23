#ifndef THREAD_H
#define THREAD_H

#include "thread_state.h"
#include "common/thread.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// MAX_THREADS is defined by CMake
#define THREAD_STACK_SIZE 1024 * STACK_SIZE
#define STACK_CANARY      0xcafebeefdeadf00d

#define THREAD_NAME_SIZE      12
#define THREAD_MSG_QUEUE_SIZE 5

typedef struct {
  int src;
  int content;
} Message;

typedef struct {
  uint8_t* stack_ptr;
  // Not an enum directly because we need to know its size
  size_t state;
  void (*signal_handler)(unsigned int);
  unsigned int pending_signal;
  int id;
  const char* name;
  // Deliberately not (void)
  void (*work)();
  ThreadArgs args;
  Message messages[THREAD_MSG_QUEUE_SIZE];
  Message* next_msg;
  Message* end_msgs;
  bool msgs_full;
  int parent;
  int child;
  uint16_t permissions;
  // Not "errno" so that we don't clash with the macro
  int err_no;
#if CODE_PAGE_SIZE
  bool in_code_page;
#if CODE_BACKING_PAGES
  size_t code_backing_page;
#endif
#endif /* CODE_PAGE_SIZE */
  uint64_t bottom_canary;
  uint8_t stack[THREAD_STACK_SIZE];
  uint64_t top_canary;
} Thread;

int k_add_thread_from_file_with_args(const char* filename,
                                     const ThreadArgs* args,
                                     uint16_t remove_permissions);

int k_add_thread(const char* name,
                 const ThreadArgs* args,
                 void* worker,
                 uint32_t flags);

bool is_valid_thread(int tid);
int k_get_thread_id(void);
bool k_get_thread_property(int tid, size_t property,
                           void* res);
bool k_set_thread_property(int tid, size_t property,
                           const void* res);

void k_thread_wait(void);
bool k_thread_wake(int tid);
bool k_thread_cancel(int tid);

// TODO: dedupe?
void k_log_event(const char* event, ...);
bool k_get_msg(int* sender, int* message);
bool k_send_msg(int destination, int message);

void k_set_kernel_config(uint32_t enable, uint32_t disable);
uint32_t k_get_kernel_config(void);

bool k_yield(int tid, int kind);

void k_invalid_syscall(size_t arg1, size_t arg2,
                       size_t arg3, size_t arg4);

bool k_has_no_permission(uint16_t permission);

#endif /* ifdef THREAD_H */
