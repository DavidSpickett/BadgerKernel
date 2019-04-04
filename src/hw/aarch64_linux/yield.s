.set INIT,      0
.set RUNNING,   1
.set SUSPENDED, 2

.global thread_switch_initial
thread_switch_initial:
  /* Called when starting scheduler (trashing regs is fine) */
  ldr x0, =current_thread // init stack pointer to
  ldr x0, [x0]            // stack pointer of dummy thread
  ldr x0, [x0]            // so we can pass the check normally
  mov sp, x0
  bl thread_switch

.global thread_switch_alrm
thread_switch_alrm:
  /* Always set next thread to scheduler */
  ldr x0, =scheduler_thread
  ldr x1, =next_thread
  str x0, [x1]

  b thread_switch

.global thread_switch
thread_switch:
  /* Save all registers to stack */
  stp x0,  x1,  [sp, #-16]!
  stp x2,  x3,  [sp, #-16]!
  stp x4,  x5,  [sp, #-16]!
  stp x6,  x7,  [sp, #-16]!
  stp x8,  x9,  [sp, #-16]!
  stp x10, x11, [sp, #-16]!
  stp x12, x13, [sp, #-16]!
  stp x14, x15, [sp, #-16]!
  stp x16, x17, [sp, #-16]!
  stp x18, x19, [sp, #-16]!
  stp x20, x21, [sp, #-16]!
  stp x22, x23, [sp, #-16]!
  stp x24, x25, [sp, #-16]!
  stp x26, x27, [sp, #-16]!
  stp x28, x29, [sp, #-16]!
  stp x30, xzr, [sp, #-16]!

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr x10, =current_thread
  ldr x11, =next_thread

  /* save stack pointer */
  ldr x1, [x10]          // get actual address of current thread
  mov x3, sp
  str x3, [x1], #8       // save current stack pointer

  /* update state */
  ldr x4, [x1]
  mov x5, #RUNNING
  cmp x4, x5
  bne dont_update_state
  mov x4, #SUSPENDED
  str x4, [x1]
dont_update_state:

  /* Switch to new thread */
  ldr x11, [x11]         // chase to get actual address of the new thread
  str x11, [x10]         // current_thread = new_thread
  ldr x3, [x11], #8      // restore stack pointer of new thread
  mov sp, x3

  /* Check that the new thread has been run at least once */
  ldr x3, [x11]           // load thread state
  mov x4, #INIT
  cmp x3, x4

  /* either way we'll be running */
  mov x4, #RUNNING
  str x4, [x11]

  bne restore_regs

  /* Fake lr */
  ldr x30, =thread_start
  b return

restore_regs:
  ldp x30, xzr, [sp], #16 // xzr to keep alignment
  ldp x28, x29, [sp], #16
  ldp x26, x27, [sp], #16
  ldp x24, x25, [sp], #16
  ldp x22, x23, [sp], #16
  ldp x20, x21, [sp], #16
  ldp x18, x19, [sp], #16
  ldp x16, x17, [sp], #16
  ldp x14, x15, [sp], #16
  ldp x12, x13, [sp], #16
  ldp x10, x11, [sp], #16
  ldp x8,  x9,  [sp], #16
  ldp x6,  x7,  [sp], #16
  ldp x4,  x5,  [sp], #16
  ldp x2,  x3,  [sp], #16
  ldp x0,  x1,  [sp], #16

return:
  ret
