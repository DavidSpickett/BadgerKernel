.global _Reset
.thumb_func
_Reset:
  /* MSP was already set to kernel stack, from the vector table */

  // Load the address directly so the distance isn't an issue
  ldr r0, =entry
  // Note that the Thumb bit is set for us
  bx r0
