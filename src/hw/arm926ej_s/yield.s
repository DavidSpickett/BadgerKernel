.global platform_yield
.type platform_yield,%function
platform_yield:
   /* Check stack extent */
   ldr r0, =thread_stack_offset
   ldr r1, =current_thread
   ldr r0, [r0]        // chase offset
   ldr r1, [r1]        // chase current
   add r0, r1, r0      // get min. valid stack pointer
   mov r1, sp          // get current sp
   sub r1, r1, #(12*4) // take away space we want to use
   cmp r0, r1          // is potential sp < min valid sp?
   bhs stack_extent_failed

.global platform_yield_no_stack_check
platform_yield_no_stack_check:
   // Jump here when yielding into scheduler for the first time

   /* Save callee saved regs (no sp/pc) */
   push {r4-r12, r14}

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
   pop {r4-r12, r14}       // restore our own regs (no sp/pc)
   bx lr
