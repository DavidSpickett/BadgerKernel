.set INIT,      0
.set RUNNING,   1
.set SUSPENDED, 2

.global thread_switch_initial
thread_switch_initial:
  /* Called when starting the scheduler, so we can trash regs here */
  ldr r0, =current_thread // Set the actual stack pointer to
  ldr r0, [r0]            // that of the dummy thread
  ldr r0, [r0]            // so that we can pass the stack check
  mov sp, r0              // without having more than one entry point
  bl thread_switch        // to the switching code

.global thread_switch_alrm
thread_switch_alrm:
  /* Next thread is always the scheduler */
  ldr r0, =next_thread
  ldr r1, =scheduler_thread
  str r1, [r0]

  b thread_switch

.global thread_switch
thread_switch:
  /* Push all callee saved regs apart from sp and pc */
  push {r4-r12, r14} // lr also included here

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr r10, =current_thread
  ldr r11, =next_thread

  /* Save our Stack pointer */
  ldr r1, [r10]           // get actual adress of current thread
  str sp, [r1], #4

  /* update state */
  ldr r2, [r1]
  mov r3, #RUNNING
  cmp r2, r3
  bne dont_update_state
  mov r2, #SUSPENDED
  str r2, [r1]
dont_update_state:

  /* Switch to new thread */
  ldr r11, [r11]          // chase to get actual address of new thread
  str r11, [r10]          // current = to (no need to chase ptr)
  ldr sp, [r11], #4       // restore stack pointer of new thread

  /* check that this thread has been run at least once */
  ldr r3, [r11]           // get state of new thread
  mov r4, #INIT
  cmp r3, r4

  /* either way we start running */
  mov r4, #RUNNING
  str r4, [r11]

  bne restore_regs
  ldr lr, =thread_start
  b exc_return            // don't restore other regs, just use fake lr value

restore_regs:
  pop {r4-r12, r14}       // restore our own regs (no sp/pc)

exc_return:
  bx lr
