.extern current_thread
.extern next_thread
.global platform_yield
platform_yield:   
  // TODO: NEON, CPSR & SPSR
  stp x0,  x1,  [sp, -16]!
  stp x2,  x3,  [sp, -16]! 
  stp x4,  x5,  [sp, -16]!
  stp x6,  x7,  [sp, -16]!
  stp x8,  x9,  [sp, -16]!
  stp x10, x11, [sp, -16]!
  stp x12, x13, [sp, -16]!
  stp x14, x15, [sp, -16]!
  stp x16, x17, [sp, -16]!
  stp x18, x19, [sp, -16]!
  stp x20, x21, [sp, -16]!
  stp x22, x23, [sp, -16]!
  stp x24, x25, [sp, -16]!
  stp x26, x27, [sp, -16]!
  stp x28, x29, [sp, -16]!
  str x30,      [sp, -8 ]!

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr x10, =current_thread
  ldr x11, =next_thread

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

  /* Restore all registers */
  ldr x30,      [sp], #8
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

  ret
