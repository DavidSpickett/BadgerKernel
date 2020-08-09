#ifndef USER_THREAD_H
#define USER_THREAD_H

#include <stdint.h>
#include "thread_common.h"
#include "thread_state.h"

int add_thread(const char* name,
               const ThreadArgs* args,
               void* worker,
               size_t kind);

int add_named_thread(void (*worker)(void), const char* name);
int add_thread_from_worker(void (*worker)(void));
int add_thread_from_file(const char* filename);
int add_thread_from_file_with_args(const char* filename, const ThreadArgs* args);
int add_named_thread(void (*worker)(void), const char* name);
int add_named_thread_with_args(void (*worker)(), const char* name,
                               const ThreadArgs* args);

bool get_thread_property(int tid, size_t property,
                         size_t* res);
bool set_thread_property(int tid, size_t property,
                         const void* value);

// As in, the current ID
int get_thread_id(void);
bool thread_name(int tid, const char** name);
bool set_thread_name(int tid, const char* name);

void yield(void);
bool thread_join(int tid, ThreadState* state);
bool get_thread_state(int tid, ThreadState* state)
  __attribute__((nonnull));
bool set_child(int child);
bool get_child(int tid, int* child);

void log_event(const char* event, ...);

void set_kernel_config(uint32_t enable, uint32_t disable);
uint32_t get_kernel_config(void);

bool yield_to(int tid);
bool yield_next(void);

bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef USER_THREAD_H */
