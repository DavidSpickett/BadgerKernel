#ifndef USER_THREAD_H
#define USER_THREAD_H

#include <stdint.h>
#include "common/thread.h"
#include "common/trace.h"
#include "thread_state.h"

int add_thread(const char* name,
               const ThreadArgs* args,
               void* worker,
               uint32_t flags);

int add_named_thread(void (*worker)(void), const char* name);
int add_thread_from_worker(void (*worker)(void));
int add_thread_from_file(const char* filename);
int add_thread_from_file_with_args(const char* filename, const ThreadArgs* args);
int add_named_thread(void (*worker)(void), const char* name);
int add_named_thread_with_args(void (*worker)(), const char* name,
                               const ThreadArgs* args);

// TODO: it's a uint32_t so you can use the existing
// _NO_ macros, which are shifted << 16 already
// Returns the updated set of permissions
uint16_t permissions(uint32_t remove);

bool get_thread_property(int tid, size_t property,
                         void* res);
bool set_thread_property(int tid, size_t property,
                         const void* value);

bool get_thread_registers(int tid, RegisterContext* regs);
bool set_thread_registers(int tid, RegisterContext regs);

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

void thread_wait(void);
bool thread_wake(int tid);
bool thread_cancel(int tid);

bool thread_signal(int tid, unsigned int signal);
bool set_signal_handler(void (*handler)(unsigned int));

void log_event(const char* event, ...);

void set_kernel_config(uint32_t enable, uint32_t disable);
uint32_t get_kernel_config(void);

bool yield_to(int tid);
bool yield_next(void);

bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef USER_THREAD_H */
