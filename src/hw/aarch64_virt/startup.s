.global _Reset
_Reset:
 ldr x0, =stack_top
 mov sp, x0
 bl entry
 b .
