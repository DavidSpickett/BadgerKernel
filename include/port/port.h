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

typedef union {
  PlatformRegs platform_regs;
  GenericRegs generic_regs;
} RegisterContext;

void platform_init_register_context(PlatformRegs* ctx);

#endif /* ifdef PORT_PORT_H */
