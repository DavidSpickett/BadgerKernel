// TODO: CPSR/SPSR, flags?
asm volatile (
 /* Save our own state */
 "push {r0-r12, r14}\n\t"      // note no PC or sp here
 "ldr r1, =current_thread\n\t" // get *address* of current thread var
 "ldr r1, [r1]\n\t"            // get actual adress of current thread
 "str sp, [r1], #4\n\t"        // save current stack pointer to struct
 "ldr r2, =thread_return\n\t"  // get address to resume this thread from
 "str r2, [r1]\n\t"            // make that our next new PC
 
 /* Switch to new thread */
 "mov r1, %0\n\t"              // get addr of thread to move to (directly this time)
 "ldr r2, =current_thread\n\t" // set new current thread addr
 "str r1, [r2]\n\t"            // (don't need to chase ptr here)
 "ldr sp, [r1], #4\n\t"        // restore stack pointer of new thread
 "ldr pc, [r1]\n\t"            // jump to the next PC of that thread

 /* Resume this thread */
 "thread_return:\n\t"          // threads are resumed from here
 "ldr r1, =current_thread\n\t" // current will always be ourselves here
 "ldr r1, [r1]\n\t"            // get *value* of current thread ptr
 "ldr sp, [r1]\n\t"            // restore our own stack pointer
 "pop {r0-r12, r14}\n\t"       // restore our own regs
:
: "r" (to)
);
