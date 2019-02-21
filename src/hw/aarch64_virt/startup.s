.global _Reset
_Reset:
 # Set FPEN=3 to enable NEON
 mov x1, #(0x3 << 20)
 msr cpacr_el1, x1
 isb
 ldr x0, =stack_top
 mov sp, x0
 bl entry
 b .
