void platform_yield(void** current, void* to) {
  asm volatile (
   /* Setup pointers in some high reg numbers we won't overwrite */
   "mov x10, %[curr]\n\t"
   "mov x11, %[to]\n\t"

   /* Save our PC and return address */
   "ldr x1, [x10]\n\t"           // get actual adress of current thread
   "mov x3, sp\n\t"
   "str x3, [x1], %[inc]\n\t"    // save current stack pointer
   "ldr x2, =thread_return\n\t"  // get address to resume this thread from
   "str x2, [x1]\n\t"            // store that in our task struct

   /* Switch to new thread */
   "str x11, [x10]\n\t"          // current = to (no need to chase ptr)
   "ldr x3, [x11], %[inc]\n\t"   // restore stack pointer of new thread
   "mov sp, x3\n\t"
   "ldr x3, [x11]\n\t"           // jump to the PC of that thread
   "br x3\n\t"

   /* Resume this thread (note r10/r11 are still curent/to) */
   "thread_return:\n\t"          // threads are resumed from here
   "ldr x2, [x10]\n\t"           // get *value* of current thread ptr
   "ldr x3, [x2]\n\t"            // restore our own stack pointer
   "mov sp, x3\n\t"
  :
  : [curr]"r"(current), [to]"r"(to), [inc]"I"(sizeof(void*))
  : "x19", "x20", "x21", "x22", "x23", "x24",
    "x25", "x26", "x27", "x28" // x29 is done for us
  );
}
