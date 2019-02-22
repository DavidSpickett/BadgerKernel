.extern next_thread
.extern current_thread
.global platform_yield
platform_yield:
   /* Save our own state */
   push {r0-r7, lr}       // note that we'll save lr here
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
   pop {r0-r7, pc}       // restore our own regs
                         // and return in the process
