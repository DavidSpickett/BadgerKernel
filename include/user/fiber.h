#ifndef USER_FIBER_H
#define USER_FIBER_H

#include "port/port.h"

void make_context(FiberContext* ctx, void (*function)(FiberContext*),
                  uint8_t* stack_ptr);

void swap_context(FiberContext* to);
void get_context(FiberContext* ctx);
void set_context(const FiberContext* ctx);

#endif /* ifdef USER_FIBER_H */
