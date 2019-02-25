.global platform_yield_initial
platform_yield_initial:
  /* Init stack ptr to dummy thread so we can yield to scheduler */
  ldr r0, =current_thread // Set the actual stack pointer to
  ldr r0, [r0]            // that of the dummy thread
  ldr r0, [r0]            // so that we can pass the stack check
  mov sp, r0              // without having more than one entry point
  bl platform_yield       // to the switching code

.global platform_yield
platform_yield:
   /* Check stack extent */
   ldr r0, =thread_stack_offset
   ldr r1, =current_thread
   ldr r0, [r0]                 // chase offset
   ldr r1, [r1]                 // chase current
   add r0, r1, r0               // get min. valid stack pointer
   mrs r1, msp                  // get current sp
   sub r1, r1, #(5*4)           // take away space we want to use
   cmp r0, r1                   // is potential sp < min valid sp?
   ldr r2, =stack_extent_failed // can't get a relocation to this, use addr
   bls stack_check_passed       // can't conditonally branch to register...
   bx r2                        // so branch over this instr if check passed
stack_check_passed:             // carry on with the yield

   /* Save callee saved regs */
   push {r4-r7, lr}       // note that we'll save lr here
                          // and use it to return later

   /* Setup pointers some high reg numbers */
   ldr r6, =current_thread
   ldr r7, =next_thread

   /* Save our PC and return address */
   ldr r1, [r6]           // get actual adress of current thread
   mrs r5, msp            // save current stack pointer
   str r5, [r1]           //
   add r1, r1, #4         //
   ldr r2, =thread_return // get address to resume this thread from
   mov r3, #1             // make sure we jump to thumb mode
   orr r2, r2, r3         //
   str r2, [r1]           // store resume pc in our task struct

   /* Switch to new thread */
   ldr r7, [r7]          // chase to get actual address of nex thread
   str r7, [r6]          // current_thread = next_thread
   ldr r5, [r7]          // restore stack pointer of new thread
   add r7, r7, #4
   msr msp, r5           // assume that CONTROL.SPSEL is always 0
                         // (we're not using interrupts yet)
   ldr r5, [r7]          // jump to the PC of that thread
   bx r5

   /* Resume this thread (note r6 is still &current_thread here) */
   thread_return:        // threads are resumed from here
   ldr r2, [r6]          // get *value* of current thread ptr
   ldr r5, [r2]          // restore our own stack pointer
   msr msp, r5           //
   pop {r4-r7, pc}       // restore our own regs
                         // and return in the process
