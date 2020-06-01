#ifndef USER_THREAD_H
#define USER_THREAD_H

#include "thread_common.h"
#include "thread_state.h"

int add_named_thread(void (*worker)(void), const char* name);
int add_thread(void (*worker)(void));
#if CODE_PAGE_SIZE
int add_thread_from_file(const char* filename);
#endif
int add_named_thread(void (*worker)(void), const char* name);
int add_named_thread_with_args(void (*worker)(), const char* name,
                               const ThreadArgs* args);

// As in, the current ID
int get_thread_id(void);
const char* get_thread_name(void);
bool thread_name(int tid, const char** name);

void yield(void);
bool thread_join(int tid, ThreadState* state);
bool get_thread_state(int tid, ThreadState* state)
  __attribute__((nonnull));
bool set_child(int child);

void log_event(const char* event, ...);

// TODO: more extensible interface for this?
void set_kernel_config(const KernelConfig* config);

bool yield_to(int tid);
bool yield_next(void);

bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef USER_THREAD_H */
