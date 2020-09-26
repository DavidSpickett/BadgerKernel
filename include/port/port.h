#ifndef PORT_PORT_H
#define PORT_PORT_H

#ifdef __aarch64__
#include "aarch64.h"
#elif defined __thumb__
#include "thumb.h"
#elif defined __arm__
#include "arm.h"
#else
#error Unknown architecture!
#endif

void platform_init_register_context(RegisterContext* regs);
void print_register_context(const RegisterContext* ctx);

#endif /* ifdef PORT_PORT_H */
