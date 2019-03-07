.global _Reset
_Reset:
  /* Note that Qemu boots in EL1 */

  /* Set FPEN=3 to enable NEON */
  mov x1, #(0x3 << 20)
  msr cpacr_el1, x1
  isb

  /* Init stack for exceptions */
  ldr x0, =monitor_stack_top
  ldr x0, [x0]  // Chase to get actual value
  msr SPSel, #1
  mov sp, x0

  /* Setup timer
     Allow EL0 to read the registers, though it's
     only used by some GDB helper functions.
  */
  mov x0, #(1<<8) | (1<<1)    // so EL0 can at least read them
  msr CNTKCTL_EL1, x0
  ldr x0, =1000000            // 1 MHz timer freq
  msr CNTFRQ_EL0, x0

  /* Init stack for application */
  ldr x0, =stack_top
  msr SPSel, #0
  mov sp, x0

  /* Configure interrupt routing
     Uses stack but this is fine because of the setup above. */
  mov x0, #27  // Timer IRQ
  bl gic_init

  /* Make sure virtual timer is disabled at startup */
  mov x0, #2
  msr CNTV_CTL_EL0, x0

  /* Locate vector table */
  ldr x1,=el1_table
  msr vbar_el1,x1  // Qemu boots in EL1

  /* Go to entry as El0 */
  ldr x0, =entry
  msr ELR_EL1, x0
  eret

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
  b . // sync
  .balign 128
  b .  // irq
  .balign 128
  b . // fiq
  .balign 128
  b . // serror

  /* Lower EL using AArch64 */
  .balign 128
  .extern thread_switch
  b thread_switch  // sync aka svc
  .balign 128
  b handle_timer   // irq
  .balign 128
  b . // fiq
  .balign 128
  b . // serror
