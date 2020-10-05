#ifndef USER_FIBRE_H
#define USER_FIBRE_H

#include "port/port.h"

void make_context(UserContext* ctx, void (*function)(UserContext*), uint8_t* stack_ptr);

void swap_context(UserContext* to);
void get_context(UserContext* ctx);
void set_context(const UserContext* ctx);

#endif /* ifdef USER_FIBRE_H */
