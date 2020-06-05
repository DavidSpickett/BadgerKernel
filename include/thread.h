#ifndef THREAD_H
#define THREAD_H

#include "thread_state.h"
#include "thread_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_THREADS 6

#ifndef linux // TODO: linux mess
#define THREAD_STACK_SIZE 1024 * STACK_SIZE
#define STACK_CANARY      0xcafebeefdeadf00d
#endif

#define THREAD_NAME_SIZE      12
#define THREAD_MSG_QUEUE_SIZE 5

typedef struct {
  int src;
  int content;
} Message;

typedef struct {
#ifdef linux
  pthread_t self;
#else
  uint8_t* stack_ptr;
#endif
  // Not an enum directly because we need to know its size
  size_t state;
  int id;
  const char* name;
  // Deliberately not (void)
  void (*work)();
  ThreadArgs args;
  Message messages[THREAD_MSG_QUEUE_SIZE];
  Message* next_msg;
  Message* end_msgs;
  bool msgs_full;
#if CODE_PAGE_SIZE
  bool in_code_page;
#if CODE_BACKING_PAGES
  size_t code_backing_page;
#endif
#endif /* CODE_PAGE_SIZE */
#ifndef linux
  uint64_t bottom_canary;
  uint8_t stack[THREAD_STACK_SIZE];
  uint64_t top_canary;
#endif
} Thread;

int k_add_thread(void (*worker)(void));
#if CODE_PAGE_SIZE
int k_add_thread_from_file(const char* filename);
#endif
int k_add_named_thread(void (*worker)(void), const char* name);
int k_add_named_thread_with_args(void (*worker)(), const char* name,
                               const ThreadArgs* args);

bool is_valid_thread(int tid);
int k_get_thread_id(void);
const char* k_get_thread_name(void);

bool k_yield_next(void);
bool k_yield_to(int tid);
void thread_wait(void);
bool thread_wake(int tid);
bool thread_cancel(int tid);
void k_log_event(const char* event, ...);
bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

void k_set_kernel_config(const KernelConfig* config);
bool k_get_thread_state(int tid, ThreadState* state);

void k_thread_yield(Thread* next);
bool k_yield_to(int tid);

#endif /* ifdef THREAD_H */
