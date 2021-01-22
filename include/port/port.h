#ifndef PORT_PORT_H
#define PORT_PORT_H

#include "common/svc_calls.h"
#include "common/syscall.h"
#include "common/thread.h"
#include <stddef.h>
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
#ifndef ALIGN_STACK_PTR
#error Must define ALIGN_STACK_PTR!
#endif

void load_first_thread(void);

void platform_init_register_context(RegisterContext* regs);
void print_register_context(const RegisterContext* ctx);

void signal_handler_wrapper(uint32_t signal, void (*handler)(uint32_t));
void signal_handler_wrapper_end(void);

void do_svc(SVCCode code);

size_t generic_semihosting_call(size_t operation, size_t* parameters);
size_t generic_syscall(Syscall num, size_t arg1, size_t arg2, size_t arg3,
                       size_t arg4);

// Used to catch returning fibers
void set_context_from_stack_address(void);

#endif /* ifdef PORT_PORT_H */
