.global _Reset
_Reset:
  ldr r0, =monitor_stack_top // set supervisor mode stack ptr
  ldr r0, [r0]               // chase to get value
  mov sp, r0
  cps #16                    // switch to user mode

  /* Can't move the vector table because virt has
     other stuff at the 0xffff0000 address.
     so the vectors are loaded as a seperate object
     and placed at 0. */

  ldr sp, =stack_top         // set user mode stack pointer
  bl entry
