void platform_yield(void** current, void* to) {
  asm volatile (
   /* Save our own state */
   "push {r0-r12, r14}\n\t"      // note no PC or SP here

   /* Setup pointers in some high reg numbers we won't overwrite */
   "mov r10, %[curr]\n\t"
   "mov r11, %[to]\n\t"

   /* Save our PC and return address */
   "ldr r1, [r10]\n\t"           // get actual adress of current thread
   "str sp, [r1], %[inc]\n\t"    // save current stack pointer
   "ldr r2, =thread_return\n\t"  // get address to resume this thread from
   "str r2, [r1]\n\t"            // store that in our task struct

   /* Switch to new thread */
   "str r11, [r10]\n\t"          // current = to (no need to chase ptr)
   "ldr sp, [r11], %[inc]\n\t"   // restore stack pointer of new thread
   "ldr pc, [r11]\n\t"           // jump to the PC of that thread

   /* Resume this thread (note r10/r11 are still curent/to) */
   "thread_return:\n\t"          // threads are resumed from here
   "ldr r2, [r10]\n\t"           // get *value* of current thread ptr
   "ldr sp, [r2]\n\t"            // restore our own stack pointer
   "pop {r0-r12, r14}\n\t"       // restore our own regs
  :
  : [curr]"r"(current), [to]"r"(to), [inc]"I"(sizeof(void*))
  );
}
