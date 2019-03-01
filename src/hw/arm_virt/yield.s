.data
mon_stack_err: .string "Monitor stack underflow!\n"

.text

.set SYSTEM_MODE,     0x1f
.set SUPERVISOR_MODE, 0x13
.set USER_MODE,       0x10

.global platform_yield_initial
platform_yield_initial:
  /* Called when starting the scheduler, so we can trash regs here */
  ldr r0, =current_thread // Set the actual stack pointer to
  ldr r0, [r0]            // that of the dummy thread
  ldr r0, [r0]            // so that we can pass the stack check
  mov sp, r0              // without having more than one entry point
  bl platform_yield       // to the switching code

.global platform_yield
platform_yield:
  svc 0xdead
  bx lr

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
  /* Unkown svc code */
  b .

semihosting:
  pop {r0-r1}
  cps #SYSTEM_MODE // sys mode shares sp of user mode
  svc 0x123456     // picked up by Qemu
  bx lr

switch_thread:
  /* Check stack extent */
  ldr r0, =thread_stack_offset
  ldr r1, =current_thread
  ldr r0, [r0]                // chase offset
  ldr r1, [r1]                // chase current
  add r0, r1, r0              // get min. valid stack pointer
  cps #SYSTEM_MODE
  mov r1, sp                  // get thread's stack pointer
  cps #SUPERVISOR_MODE
  sub r1, r1, #((13+1+1+1)*4) // 13 gp regs plus lr plus lr+CPSR
  cmp r0, r1                  // is potential sp < min valid sp?
  bhs stack_extent_failed     // call C function to error and exit

  /* Start switching thread */
  pop {r0, r1}            // pop temps from supervisor stack
  srsdb sp!, #SYSTEM_MODE // save CPSR and lr to system mode stack pointer
  cps #SYSTEM_MODE        // continue in system mode

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
  ldr r4, =thread_start
  cmp r3, r4
  bne restore_regs

  /* if not we need to fake the lr and CPSR being on the stack */
  str r4, [sp, #-4]!      // link register
  mov r5, #USER_MODE      // not sure of mode but flags should be 0
  stmdb sp!, {r4,r5}
  b exc_return            // don't restore other regs, just these two when we return

restore_regs:
  pop {r0-r12, r14}       // restore our own regs (no sp/pc)

exc_return:
  rfeia sp!               // restore CPSR and lr then eret
