#include "port/port.h"

void platform_init_register_context(RegisterContext* regs) {
  // Must run in Thumb mode
  regs->xpsr = 1 << 24;
}
