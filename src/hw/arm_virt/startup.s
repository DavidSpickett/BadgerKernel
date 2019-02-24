.global _Reset
_Reset:
  mrs r0, cpsr        // set the cpu to user mode
  bic r0, r0, #0xf    // clear bottom 4 to go from 0x13->0x10
  msr cpsr, r0

  /* Can't move the vector table because virt has
     other stuff at the 0xffff0000 address.
     so the vectors are loaded as a seperate object. */

  ldr sp, =stack_top
  bl entry
