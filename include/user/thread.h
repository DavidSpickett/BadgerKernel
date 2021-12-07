#ifndef USER_THREAD_H
#define USER_THREAD_H

#include "common/macros.h"
#include "common/thread.h"
#include "common/trace.h"
#include <stdint.h>

BK_EXPORT int add_thread(const char* name, const ThreadArgs* args, void* worker,
                         const ThreadFlags* flags);

BK_EXPORT int add_named_thread(void (*worker)(void), const char* name);
BK_EXPORT int add_thread_from_worker(void (*worker)(void));
BK_EXPORT int add_thread_from_file(const char* filename);
BK_EXPORT int add_thread_from_file_with_args(const char* filename,
                                             const ThreadArgs* args);
BK_EXPORT int add_named_thread(void (*worker)(void), const char* name);
BK_EXPORT int add_named_thread_with_args(void (*worker)(), const char* name,
                                         const ThreadArgs* args);

// Remove "remove" from the current set and return the new permissions
BK_EXPORT uint16_t permissions(uint16_t remove);

BK_EXPORT bool get_thread_property(int tid, size_t property, void* res);
BK_EXPORT bool set_thread_property(int tid, size_t property, const void* value);

BK_EXPORT bool get_thread_registers(int tid, RegisterContext* regs);
BK_EXPORT bool set_thread_registers(int tid, RegisterContext regs);

// As in, the current ID
BK_EXPORT int get_thread_id(void);
BK_EXPORT bool thread_name(int tid, char* name);
BK_EXPORT bool set_thread_name(int tid, const char* name);

BK_EXPORT bool yield(void);
BK_EXPORT bool yield_to(int tid);
BK_EXPORT bool thread_join(int tid, ThreadState* state);
BK_EXPORT bool get_thread_state(int tid, ThreadState* state)
    __attribute__((nonnull));
BK_EXPORT bool set_child(int child);
BK_EXPORT bool get_child(int tid, int* child);
BK_EXPORT bool get_parent(int tid, int* parent);

BK_EXPORT void thread_wait(void);
BK_EXPORT bool thread_wake(int tid);
BK_EXPORT bool thread_cancel(int tid);

BK_EXPORT bool thread_signal(int tid, uint32_t signal);
BK_EXPORT bool set_signal_handler(void (*handler)(uint32_t));

BK_EXPORT void log_event(const char* event, ...);

BK_EXPORT void set_kernel_config(uint32_t enable, uint32_t disable);
BK_EXPORT uint32_t get_kernel_config(void);

BK_EXPORT bool get_msg(int* sender, int* message);
BK_EXPORT bool send_msg(int destination, int message);

#endif /* ifdef USER_THREAD_H */
