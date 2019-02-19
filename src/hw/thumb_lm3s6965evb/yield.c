void platform_yield(void** current, void* to) {
  asm volatile (
   /* Save our own state */
   "push {r0-r12, r14}\n\t"      // note no PC or SP here

   /* Setup pointers in some high reg numbers we won't overwrite */
   "mov r10, %[curr]\n\t"
   "mov r11, %[to]\n\t"

   /* Save our PC and return address */
   "ldr r1, [r10]\n\t"           // get actual adress of current thread
   "mrs r12, msp\n\t"            // save current stack pointer
   "str r12, [r1], %[inc]\n\t"   //
   "ldr r2, =thread_return\n\t"  // get address to resume this thread from
   "orr r2, r2, #1\n\t"          // make sure we jump to thumb mode
   "str r2, [r1]\n\t"            // store that in our task struct

   /* Switch to new thread */
   "str r11, [r10]\n\t"          // current = to (no need to chase ptr)
   "ldr r12, [r11], %[inc]\n\t"  // restore stack pointer of new thread
   "msr msp, r12\n\t"            // assume that CONTROL.SPSEL is always 0
                                 // (we're not using interrupts yet)
   "ldr r12, [r11]\n\t"          // jump to the PC of that thread
   "bx r12\n\t"

  // /* Resume this thread (note r10/r11 are still curent/to) */
   "thread_return:\n\t"          // threads are resumed from here
   "ldr r2, [r10]\n\t"           // get *value* of current thread ptr
   "ldr r12, [r2]\n\t"           // restore our own stack pointer
   "msr msp, r12\n\t"            //
   "pop {r0-r12, r14}\n\t"       // restore our own regs
  :
  : [curr]"r"(current), [to]"r"(to), [inc]"I"(sizeof(void*))
  );
}
