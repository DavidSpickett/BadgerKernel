#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"

#define GIC_BASE  0x08000000
#define GICD_BASE GIC_BASE             // Distributor
#define GICC_BASE (GIC_BASE + 0x10000) // CPU interface

#define TIMER_IRQ 27

#define PTR(x) (volatile uint32_t*)(x)

#define CPU_CTLR PTR(GICC_BASE)         // CPU interface control
#define CPU_PMR  PTR(GICC_BASE + 0x004) // Priority mask
#define CPU_BPR  PTR(GICC_BASE + 0x008) // Binary point

#define DIST_CTLR          PTR(GICD_BASE)                     // Distributor control
#define DIST_ISENABLER(n)  PTR(GICD_BASE + 0x100 + ( n * 4 )) // Set enable
#define DIST_IPRIORITYR(n) PTR(GICD_BASE + 0x400 + ( n * 4 )) // Priority
#define DIST_ITARGETSR(n)  PTR(GICD_BASE + 0x800 + ( n * 4 )) // Processor targets

void enable_timer() {
  asm volatile ("svc 1");
}

void disable_timer() {
  asm volatile ("svc 0");
}

// GDB helpers
size_t rtc() {
  size_t res;
  asm volatile ("mrs %0, CNTV_CTL_EL0"
    : "=r"(res)
  );
  return res;
}

size_t rt() {
  size_t res;
  asm volatile ("mrs %0, CNTVCT_EL0"
    : "=r"(res)
  );
  return res;
}

/* I'm assuming this is called from EL1,
   with some valid stack to use.  */
void gic_init(void)
{
  /* CPU Interface */

  // Disable
  *CPU_CTLR = 0;

  // Set the priority filter level as the lowest priority
  // (higher no. = lower priority)
  *CPU_PMR = 0xFF;

  // Handle all interrupts as a single group
  *CPU_BPR = 0x0;

  // Re-enable
  *CPU_CTLR = 1;

  /* Distributor */

  // Disable
  *DIST_CTLR = 0;

  // 0 aka highest priority, 4, 8 bit fields per register
  *DIST_IPRIORITYR(TIMER_IRQ/4) = 0;

  // 4, 8 bit fields per processor target register
  uint32_t shift = (TIMER_IRQ % 4) * 8;
  // send interrupt to 1 aka CPU 0 interface
  *DIST_ITARGETSR(TIMER_IRQ/4) = 1 << shift;

  // Enable interrupt (one bit per)
  *DIST_ISENABLER(TIMER_IRQ/32) = 1 << TIMER_IRQ;

  // Re-enable
  *DIST_CTLR = 1;
}
