.global _Reset
_Reset:
 # Note that Qemu boots in EL1

 # Set FPEN=3 to enable NEON
 mov x1, #(0x3 << 20)
 msr cpacr_el1, x1
 isb

 # Init stack for exceptions
 ldr x0, =monitor_stack_top
 ldr x0, [x0]  // Chase to get actual value
 MSR SPSel, #1
 mov sp, x0

 # Init stack for application
 ldr x0, =stack_top
 MSR SPSel, #0
 mov sp, x0

 bl enable_irq

 # Go to entry as El0
 ldr x0, =entry
 msr ELR_EL1, x0
 eret

enable_irq:
  ldr x1,=el1_table
  // Qemu boots in EL1
  msr vbar_el1,x1
  // Just IRQs
  mov x1, #(1<<7)
  msr daif, x1
  ret

/* Current EL with SP0 */
.balign 0x800
el1_table:
b . // sync
.balign 128
b . // irq
.balign 128
b . // fiq
.balign 128
b . // serror

/* Current EL with SPxELR_EL3 */
.balign 128
/* We end up here if we do something wrong in thread_switch.
  Go back there and we'll detect that and exit. */
b thread_switch
.balign 128
b . // irq
.balign 128
b . // fiq
.balign 128
b . // serror

/* Lower EL using AArch64 */
.balign 128
.extern thread_switch
b thread_switch  // sync aka svc
.balign 128
b . // irq
.balign 128
b . // fiq
.balign 128
b . // serror
