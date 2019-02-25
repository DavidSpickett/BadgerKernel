.data
// TODO: dedupe these strings
           mon_stack_err: .string "Monitor stack underflow!\n"
unknown_monitor_call_err: .string "Got an unexpected monitor call!\n"

.text
.global platform_yield
platform_yield:
  svc 0xdead

.global __platform_yield
__platform_yield:
  push {r0-r1}     // push to monitor stack

  /* Check monitor stack. (note that we don't use all of it on Arm */
  ldr r0, =monitor_stack_top
  ldr r0, [r0]
  sub r0, r0, #8   // 2 Arm registers
  mov r1, sp
  cmp r0, r1
  beq monitor_stack_ok
  ldr r0, =mon_stack_err
  bl qemu_print
  b qemu_exit
monitor_stack_ok:

  /* See if this is semihosting or a thread switch */
  ldr r0, [lr, #-4]        // load svc instruction used
  ldr r1, =0xFFFFFF        // mask to get code
  and r0, r0, r1           //
  ldr r1, =0x00dead        // thread switch
  cmp r0, r1
  beq switch_thread
  ldr r1, =0x123456        // semihosting call
  cmp r0, r1
  beq semihosting
  ldr r1, =unknown_monitor_call_err
  b qemu_print
  b qemu_exit

semihosting:
  pop {r0-r1}
  cps #0x1f    // system mode (shares sp of user)
  svc 0x123456 // picked up by Qemu
  bx lr

switch_thread:
  /* Check stack extent */
  ldr r0, =thread_stack_offset
  ldr r1, =current_thread
  ldr r0, [r0]        // chase offset
  ldr r1, [r1]        // chase current
  add r0, r1, r0      // get min. valid stack pointer
  cps #0x1f           // go to system mode
  mov r1, sp          // get thread's stack pointer
  cps #0x13           // back to svc mode
  sub r1, r1, #(12*4) // take away space we want to use
  cmp r0, r1          // is potential sp < min valid sp?
  bhs stack_extent_failed

.global platform_yield_no_stack_check
platform_yield_no_stack_check:
  // Jump here when yielding into scheduler for the first time

  pop {r0, r1} // pop temps from supervisor stack
  cps #16      // switch to user mode

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
