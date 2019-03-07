.global _Reset
_Reset:
  ldr r0, =monitor_stack_top // set supervisor mode stack ptr
  ldr r0, [r0]               // chase to get value
  mov sp, r0

  /* Allow phyiscal timer reads from user mode */
  ldr r0, =(3 | (3<<8))
  mcr p15, 0, r0, c14, c1, 0 // CNTKCTL

  /* Set 1MHz frequency */
  ldr r0, =1000000
  mcr p15, 0, r0, c14, c0, 0 // CNTFRQ

  /* Make sure physical timer is disabled */
  mov r0, #2                 // disable, mask interrupt
  mcr p15, 0, r0, c14, c2, 1 // CNTP_CTL

  /* According to Arm developer docs, physical timer
   is 26, virtual is 27. According to:
   https://github.com/littlekernel/lk/issues/54
   it depends on the Qemu version. For mine, it's 30.
  */
  mov r0, #30
  b gic_init

/* At O3 the compiler decides to use the lr in a naked
   function. So we don't rely on lr, branch back here directly.
*/
.global gic_init_ret
gic_init_ret:

  cps #16                    // switch to user mode

  /* Can't move the vector table because virt has
     other stuff at the 0xffff0000 address.
     so the vectors are loaded as a seperate object
     and placed at 0. */

  ldr sp, =stack_top         // set user mode stack pointer
  bl entry
