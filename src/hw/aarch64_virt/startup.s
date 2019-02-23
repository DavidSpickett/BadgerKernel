.global _Reset
_Reset:
 # Set FPEN=3 to enable NEON
 mov x1, #(0x3 << 20)
 msr cpacr_el1, x1
 isb
 # Init stack
 ldr x0, =stack_top
 mov sp, x0
 bl enable_irq
 bl entry
 b .

enable_irq:
  ldr x1,=el1_table
  // Qemu boots in EL1
  msr vbar_el1,x1
  // Just IRQs
  mov x1, #(1<<7)
  msr daif, x1
  ret

.balign 0x800
el1_table:
b . // sync
.balign 128
b . // irq
.balign 128
b . // fiq
.balign 128
b . // serror
.balign 128
.extern thread_switch
b thread_switch
