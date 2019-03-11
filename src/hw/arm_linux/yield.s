.set SYSTEM_MODE,     0x1f
.set SUPERVISOR_MODE, 0x13
.set USER_MODE,       0x10

.global thread_switch_initial
thread_switch_initial:
  /* Called when starting the scheduler, so we can trash regs here */
  ldr r0, =current_thread // Set the actual stack pointer to
  ldr r0, [r0]            // that of the dummy thread
  ldr r0, [r0]            // so that we can pass the stack check
  mov sp, r0              // without having more than one entry point
  bl thread_switch        // to the switching code

.global thread_switch
thread_switch:
  /* Push all regs apart from sp and pc) */
  push {r0-r12, r14} // lr also included here

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr r10, =current_thread
  ldr r11, =next_thread

  /* Save our Stack pointer and PC */
  ldr r1, [r10]           // get actual adress of current thread
  str sp, [r1], #4        // save current stack pointer
  str lr, [r1]            // save PC

  /* Switch to new thread */
  ldr r11, [r11]          // chase to get actual address of new thread
  str r11, [r10]          // current = to (no need to chase ptr)
  ldr sp, [r11], #4       // restore stack pointer of new thread

  /* check that this thread has been run at least once
     if it hasn't it's PC will be exactly thread start */
  ldr r3, [r11]           // get pc of new thread
  mov lr, r3              // we'll use it either way
  ldr r4, =thread_start
  cmp r3, r4
  bne restore_regs

  b exc_return            // don't restore other regs, just use loaded lr from above

restore_regs:
  pop {r0-r12, r14}       // restore our own regs (no sp/pc)

exc_return:
  bx lr
