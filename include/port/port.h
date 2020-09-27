#ifndef PORT_PORT_H
#define PORT_PORT_H

#include "common/thread_state.h"
#include <stdint.h>

#ifdef __aarch64__
#include "aarch64.h"
#elif defined __thumb__
#include "thumb.h"
#elif defined __arm__
#include "arm.h"
#else
#error Unknown architecture!
#endif

#ifndef PC_ADD_MODE
#error Must define PC_ADD_MODE!
#endif
#ifndef PC_REMOVE_MODE
#error Must define PC_REMOVE_MODE!
#endif
#ifndef YIELD_ASM
#error Must define YIELD_ASM!
#endif
#ifndef BRANCH_INSTR
#error Must define BRANCH_INSTR!
#endif

void platform_init_register_context(RegisterContext* regs);
void print_register_context(const RegisterContext* ctx);

void signal_handler_wrapper(uint32_t signal, void (*handler)(uint32_t));
void signal_handler_wrapper_end(void);

void do_svc(SVCCode code);

size_t generic_semihosting_call(size_t operation, size_t* parameters);

#endif /* ifdef PORT_PORT_H */
