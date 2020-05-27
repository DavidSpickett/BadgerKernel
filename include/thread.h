#ifndef THREAD_H
#define THREAD_H

#include "thread_state.h"
#include <stdbool.h>

#define MAX_THREADS 12

typedef struct {
  bool destroy_on_stack_err;
  bool log_scheduler;
} MonitorConfig;
extern MonitorConfig config;

typedef struct {
  void* a1;
  void* a2;
  void* a3;
  void* a4;
} ThreadArgs;

#define make_args(a, b, c, d)                                                  \
  { (void*)a, (void*)b, (void*)c, (void*)d }

int add_thread(void (*worker)(void));
#if CODE_PAGE_SIZE
int add_thread_from_file(const char* filename);
#endif
int add_named_thread(void (*worker)(void), const char* name);
int add_named_thread_with_args(void (*worker)(), const char* name,
                               ThreadArgs args);

bool is_valid_thread(int tid);
int get_thread_id(void);
const char* get_thread_name(void);

void yield(void);
bool yield_next(void);
bool yield_to(int tid);
void thread_wait(void);
bool thread_wake(int tid);
bool thread_cancel(int tid);
bool thread_join(int tid, ThreadState* state);
void log_event(const char* event);
bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef THREAD_H */
