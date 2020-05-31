#ifndef THREAD_H
#define THREAD_H

#include "thread_state.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX_THREADS 6

typedef struct {
  bool destroy_on_stack_err;
  bool log_scheduler;
} MonitorConfig;
extern MonitorConfig config;

typedef struct {
  size_t a1;
  size_t a2;
  size_t a3;
  size_t a4;
} ThreadArgs;

#define make_args(a, b, c, d)                   \
  { (size_t)a, (size_t)b, (size_t)c, (size_t)d }

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
void log_event(const char* event, ...);
bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef THREAD_H */
