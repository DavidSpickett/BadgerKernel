.global platform_yield
.thumb_func
platform_yield:
  svc 0xFF
  bx lr

.global platform_yield_initial
.thumb_func
platform_yield_initial:
  /* Init stack ptr to dummy thread so we can yield to scheduler */
  ldr r0, =current_thread // Set the actual stack pointer to
  ldr r0, [r0]            // that of the dummy thread
  ldr r0, [r0]            // so that we can pass the stack check
  mov sp, r0              // without having more than one entry point
  bl platform_yield       // to the switching code
  bx lr

.global __platform_yield
.thumb_func
__platform_yield:
   /* Stack pointer is MSP aka monitor stack here.
      Switch to privileged mode, but use the PSP,
      since we don't need monitor stack.
   */
   mrs r0, control
   mov r1, #1      // privileged mode
   mvn r1, r1
   and r0, r0, r1
   msr control, r0
   isb

   /* r0-3, r12, lr, pc xPSR have already been saved
      on the PSP stack. So we won't be using the monitor
      stack at all and don't need to validate it here.
   */

   /* See if this is a thread switch or a semihosting call */
   mrs r0, psp
   mov r1, #(6*4)
   add r0, r1      // find the PC we came from
   ldr r0, [r0]    // (there's no mode bit to remove)
   sub r0, r0, #2  // back one instruction to the svc
   ldr r0, [r0]    // load svc instruction
   mov r1, #0xff   // mask out the svc number
   and r0, r1, r0

   mov r1, #0xff   // thread switch
   cmp r0, r1
   beq __thread_switch
   mov r1, #0xab   // semihosting call
   cmp r0, r1
   beq semihosting
   /* Unkown svc code */
   /* Not much but enough to bkpt on in GDB */
   b .

semihosting:
   /* We need to use the PSP here and the thread's
      r0-r3 because that will contain the semihosting
      call args */
   mrs r0, psp
   mov sp, r0         // MSP = PSP (we can always reload monitor_stack_top)
   ldr r0, [sp]       // pop r0
   add sp, #4         // no direct post increment in thumb
   ldr r1, [sp]       // pop r1
   add sp, #4
   ldr r2, [sp]       // pop r2
   add sp, #4
   ldr r3, [sp]       // pop r3
   add sp, #4
   add sp, sp, #(4*4) // skip other stacked regs

   /* let's assume the psp didn't need alignent before
      regs were stacked */
   bkpt 0xab          // actual semihosting call, handled by Qemu

   /* We don't use the monitor stack but we might as well
      keep it consistent. */
   ldr r0, =monitor_stack_top
   ldr r0, [r0]

   bx lr              // return to thread

__thread_switch:
   /* Check stack extent
      Hardware has already stacked r0-3 for us. So we want
      space to save r4-r7. If the automatic saving also caused
      an underflow we'll detect it too.
   */
   ldr r0, =thread_stack_offset
   ldr r1, =current_thread
   ldr r0, [r0]                 // chase offset
   ldr r1, [r1]                 // chase current
   add r0, r1, r0               // get min. valid stack pointer
   mrs r1, psp                  // get current sp
   sub r1, r1, #(4*4)           // take away (extra) space we want to use
   cmp r0, r1                   // is potential sp < min valid sp?
   ldr r2, =stack_extent_failed // can't get a relocation to this, use addr
   bls stack_check_passed       // can't conditonally branch to register...
   bx r2                        // so branch over this instr if check passed
stack_check_passed:             // carry on with the yield

   mrs r0, psp
   mov sp, r0   // MSP = PSP

   /* callee saved regs */
   push {r4-r7} // no lr, it's already on the stack

   /* Setup pointers some high reg numbers */
   ldr r6, =current_thread
   ldr r7, =next_thread

   /* Save stack pointer */
   ldr r1, [r6]           // get actual adress of current thread
   mov r5, sp             // save current stack pointer
   str r5, [r1]           //
   add r1, r1, #4         //

   /* Save return address */
   mov r2, sp
   add r2, #(4+4+3)       // point to saved PC
   ldr r2, [r2]           // load return address
   mov r3, #1             // add thumb mode bit (not required atm but let's be safe)
   orr r2, r3, r2         //
   str r2, [r1]           // store to thread struct

   /* Switch to new thread */
   ldr r7, [r7]          // chase to get actual address of next thread
   str r7, [r6]          // current_thread = next_thread
   ldr r5, [r7]          // restore stack pointer of new thread
   mov sp, r5            // MSP = thread stack pointer
   add r7, #4
   /* no need to restore PC, exc return will do that */

   /* check that thread has been scheduled at least once already */
   ldr r3, [r7]          // load last PC
   ldr r4, =thread_start
   cmp r3, r4
   bne restore_regs

   /* If it's never been scheduled we need to fake PC and EPSR
      values being on the stack. (lr doesn't matter) */
   sub sp, #(8*4)       // size of the expected return frame
   str r4, [sp, #(6*4)] // store intial pc
   mov r1, #1           // Set Thumb bit in EPSR
   lsl r1, #24
   str r1, [sp, #(7*4)] // store intial EPSR
   b exc_return

restore_regs:
   pop {r4-r7}

exc_return:
   /* Set the psp *after* we've unstacked r4-r7.
      So it's where the automatic unstack expects it to be.
   */
   mov r0, sp
   msr psp, r0

   /* We don't use the monitor stack but we might as well
      keep it consistent. */
   ldr r0, =monitor_stack_top
   ldr r0, [r0]

   bx lr
