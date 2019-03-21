# Vectors are in vectors.s

.global _Reset
.thumb_func
_Reset:
  mrs r0, control
  mov r1, #3      // Unprivileged, using PSP
  orr r0, r1, r0
  msr control, r0
  isb

  /* MSP was set to monitor stack, from the vector table,
     not that we need it on Cortex-M */
  ldr r0, =stack_top // Setup PSP
  mov sp, r0

  // Load the address directly so the distance isn't an issue
  ldr r0, =entry
  // Note that the Thumb bit is set for us
  bx r0
