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
 ldr r0, =stack_top
 mov sp, r0
 b entry
