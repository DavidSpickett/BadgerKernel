#ifndef USER_FIBER_H
#define USER_FIBER_H

#include "common/macros.h"
#include "port/port.h"

BK_EXPORT void make_context(FiberContext* ctx, void (*function)(FiberContext*),
                            uint8_t* stack_ptr);

BK_EXPORT void swap_context(FiberContext* to);
BK_EXPORT void get_context(FiberContext* ctx);
BK_EXPORT void set_context(const FiberContext* ctx);

#endif /* ifdef USER_FIBER_H */
