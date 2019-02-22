.global platform_yield
.type platform_yield,%function
platform_yield:
   /* Save our own state */
   push {r0-r12, r14}      // note no PC or SP here

   /* Setup pointers in some high reg numbers we won't overwrite */
   ldr r10, =current_thread
   ldr r11, =next_thread

   /* Save our PC and return address */
   ldr r1, [r10]           // get actual adress of current thread
   str sp, [r1], #4        // save current stack pointer
   ldr r2, =thread_return  // get address to resume this thread from
   str r2, [r1]            // store that in our task struct

   /* Switch to new thread */
   ldr r11, [r11]          // chase to get actual address of new thread
   str r11, [r10]          // current = to (no need to chase ptr)
   ldr sp, [r11], #4       // restore stack pointer of new thread
   ldr pc, [r11]           // jump to the PC of that thread

   /* Resume this thread (note r10 is still &curent_thread here) */
   thread_return:          // threads are resumed from here
   ldr r2, [r10]           // get *value* of current thread ptr
   ldr sp, [r2]            // restore our own stack pointer
   pop {r0-r12, r14}       // restore our own regs
   bx lr
