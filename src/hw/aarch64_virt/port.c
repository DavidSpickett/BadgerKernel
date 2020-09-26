#include "port/port.h"

void platform_init_register_context(PlatformRegs* regs) {
  // Run in EL0
  regs->spsr_el1 = 0;
}
