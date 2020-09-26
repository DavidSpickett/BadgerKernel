#include "port/port.h"

void platform_init_register_context(PlatformRegs* regs) {
  // Run in user mode
  regs->cpsr = 0x10;
}
