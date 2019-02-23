.global thread_switch
thread_switch:
  // Note that all of the program runs at EL1 so sp = SP_EL1

  /* Save all registers to stack */
  stp x0,  x1,  [sp, #-16]!

  mrs x0, FPSR              // Restore these second to last 
  mrs x1, SPSR_EL1          // so we have temp regs x0/x1 to msr from
  stp x0, x1,   [sp, #-16]!

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

  /* Save our PC and return address */
  ldr x1, [x10]          // get actual adress of current thread
  mov x3, sp             //
  str x3, [x1], #8       // save current stack pointer
  mrs x2, ELR_EL1        // get address to resume this thread from
  str x2, [x1]           // store that in our task struct

  /* Switch to new thread */
  ldr x11, [x11]         // chase to get actual address of the new thread
  str x11, [x10]         // current_thread = new_thread
  ldr x3, [x11], #8      // restore stack pointer of new thread
  mov sp, x3             //
  
  // Load ELR_EL1 with the next PC of the new thread
  ldr x3, [x11]          // 
  msr ELR_EL1, x3        // when we eret we'll be in the new thread

  /* Check that the new thread has been run at least once.
     If it hasn't then there's no state to restore. */

  // For now we're going to check if it's current PC is start thread
  // That's not going to be asynochronous exception safe though
  // Bottom bit of pointer? Argh but thumb! (bit 2?)
  ldr x4, =thread_start
  cmp x3, x4 
  beq exc_return

  /* Restore all registers of the new thread */
  ldp x30, xzr, [sp], #16 //to keep alignment
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

  ldp x0,  x1,  [sp], #16 // This is FPSR/PSR not actually x0/x1
  msr FPSR, x0
  msr SPSR_EL1, x1

  ldp x0,  x1,  [sp], #16 // actual x0/x1 values

exc_return:
  // Return to PC that's in ELR_EL1
  eret 
