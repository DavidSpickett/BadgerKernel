#include <stdint.h>

/*
  Although I'm sure there are differences between the
  v2 and v3 GIC they don't affect us here. Both Arm
  and AArch64 are using the virt board so the addresses
  stay the same.
*/

#define GIC_BASE  0x08000000
#define GICD_BASE GIC_BASE             // Distributor
#define GICC_BASE (GIC_BASE + 0x10000) // CPU interface

#define PTR(x) (volatile uint32_t*)(uintptr_t)(x)

#define CPU_CTLR PTR(GICC_BASE)         // CPU interface control
#define CPU_PMR  PTR(GICC_BASE + 0x004) // Priority mask
#define CPU_BPR  PTR(GICC_BASE + 0x008) // Binary point

#define DIST_CTLR          PTR(GICD_BASE) // Distributor control
#define DIST_ISENABLER(n)  PTR(GICD_BASE + 0x100 + (n * 4)) // Set enable
#define DIST_IPRIORITYR(n) PTR(GICD_BASE + 0x400 + (n * 4)) // Priority
#define DIST_ITARGETSR(n)  PTR(GICD_BASE + 0x800 + (n * 4)) // Processor targets

// On AArch64 we have to be careful about how we call it
#if __ARM_ARCH_7A__
__attribute__((naked))
#endif
void gic_init(unsigned irq_no)
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
  *DIST_IPRIORITYR(irq_no / 4) = 0;

  // 4, 8 bit fields per processor target register
  uint32_t shift = (irq_no % 4) * 8;
  // send interrupt to 1 aka CPU 0 interface
  *DIST_ITARGETSR(irq_no / 4) = 1 << shift;

  // Enable interrupt (one bit per)
  *DIST_ISENABLER(irq_no / 32) = 1 << irq_no;

  // Re-enable
  *DIST_CTLR = 1;

#if __ARM_ARCH_7A__
  /* Since this is a naked function we need to do our
     own return. However, the compiler may decide to use
     the link register and corrupt it (at -O3).
     So just branch to a global label instead.
  */
  asm volatile("b gic_init_ret");
#endif
}
