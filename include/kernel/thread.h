#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL

#include "common/thread.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// MAX_THREADS is defined by CMake
#define THREAD_STACK_SIZE     1024 * STACK_SIZE
#define STACK_CANARY          0xcafebeefdeadf00d
#define THREAD_MSG_QUEUE_SIZE 5

typedef struct {
  int src;
  int content;
} Message;

typedef struct {
  // Only stack_ptr is read from assembly. So it is first for convenience.
  uint8_t* stack_ptr;
  ThreadState state;
  void (*signal_handler)(uint32_t);
  uint32_t pending_signals;
  int id;
  char name[THREAD_NAME_SIZE];
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
  // Where "top" is the higher address
  uint64_t top_canary;
} Thread;

extern Thread all_threads[MAX_THREADS];
extern Thread* current_thread;
extern Thread* next_thread;

#ifdef CODE_PAGE_SIZE
extern uint8_t code_page[CODE_PAGE_SIZE];
#endif

int k_add_thread_from_file_with_args(const char* filename,
                                     const ThreadArgs* args,
                                     uint16_t remove_permissions);

int k_add_thread(const char* name, const ThreadArgs* args, void* worker,
                 const ThreadFlags* flags);

void k_restart(void* worker, const char* name, const ThreadArgs* args,
               uint16_t remove_permissions);

bool is_valid_thread(int tid);
int k_get_thread_id(void);
void k_set_thread_name(Thread* thread, const char* name);
bool k_get_thread_property(int tid, size_t property, void* res);
bool k_set_thread_property(int tid, size_t property, const void* res);

void k_thread_wait(void);
bool k_thread_wake(int tid);
bool k_thread_cancel(int tid);

// TODO: dedupe?
void k_log_event(const char* event, ...);
bool k_get_msg(int* sender, int* message);
bool k_send_msg(int destination, int message);

extern uint32_t kernel_config;
void k_set_kernel_config(uint32_t enable, uint32_t disable);

bool k_yield(int tid, int kind);

bool k_has_no_permission(uint16_t permission);

void check_signals(Thread* thread);
void thread_start(void);
void init_register_context(Thread* thread);

void k_update_user_thread_info(Thread* thread);

// Not thread related but no better place for it
void k_exit(int status);

#ifdef __cplusplus
}
#endif

#endif /* ifdef KERNEL_THREAD_H */
