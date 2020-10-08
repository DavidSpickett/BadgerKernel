#ifndef USER_FIBRE_H
#define USER_FIBRE_H

#include "port/port.h"

void make_context(FibreContext* ctx, void (*function)(FibreContext*),
                  uint8_t* stack_ptr);

void swap_context(FibreContext* to);
void get_context(FibreContext* ctx);
void set_context(const FibreContext* ctx);

#endif /* ifdef USER_FIBRE_H */
