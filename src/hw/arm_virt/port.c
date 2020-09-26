#include "port/port.h"

void platform_init_register_context(RegisterContext* regs) {
  // Run in user mode
  regs->cpsr = 0x10;
}
