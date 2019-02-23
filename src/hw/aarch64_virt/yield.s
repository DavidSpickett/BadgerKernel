.extern current_thread
.extern next_thread
.global platform_yield
platform_yield:
  /* Check stack extent */
  ldr x0, =thread_stack_offset
  ldr x1, =current_thread
  ldr x0, [x0]           // chase it
  ldr x1, [x1]           // chase current thread too
  add x0, x1, x0         // get minimum valid stack pointer
  mov x1, sp             // get current sp
  sub x1, x1, #(12*64)   // take away space we want to use
  cmp x0, x1             // is potential sp < min valid sp?
  b.hs stack_extent_failed

.global platform_yield_no_stack_check
platform_yield_no_stack_check:
  // Jump here when yielding into the scheduler for the first time

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr x10, =current_thread
  ldr x11, =next_thread

  // Callee saved, no Neon/FPSR/CPSR
  stp x19, x20, [sp, -16]!
  stp x21, x22, [sp, -16]!
  stp x23, x24, [sp, -16]!
  stp x25, x26, [sp, -16]!
  stp x27, x28, [sp, -16]!
  stp x29, x30, [sp, -16]!

  /* Save our PC and return address */
  ldr x1, [x10]          // get actual adress of current thread
  mov x3, sp             //
  str x3, [x1], #8       // save current stack pointer
  ldr x2, =thread_return // get address to resume this thread from
  str x2, [x1]           // store that in our task struct

  /* Switch to new thread */
	ldr x11, [x11]         // chase to get actual address of the new thread
  str x11, [x10]         // current_thread = new_thread
  ldr x3, [x11], #8      // restore stack pointer of new thread
  mov sp, x3             //
  ldr x3, [x11]          // jump to the PC of that thread
  br x3                  //

  /* Resume this thread (note x10 is still current_thread) */
  thread_return:         // threads are resumed from here
  ldr x2, [x10]          // get *value* of current thread ptr
  ldr x3, [x2]           // restore our own stack pointer
  mov sp, x3             //

  /* Restore all callee saved registers */
  ldp x29, x30, [sp], #16
  ldp x27, x28, [sp], #16
  ldp x25, x26, [sp], #16
  ldp x23, x24, [sp], #16
  ldp x21, x22, [sp], #16
  ldp x19, x20, [sp], #16

  ret
