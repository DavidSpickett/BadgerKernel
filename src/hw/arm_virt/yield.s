.set SYSTEM_MODE,     0x1f
.set SUPERVISOR_MODE, 0x13
.set USER_MODE,       0x10

.set INIT,      0
.set RUNNING,   1
.set SUSPENDED, 2

.macro DISABLE_TIMER
  mov r0, #2                 // Disable timer and mask interrupt
  mcr p15, 0, r0, c14, c2, 1 // CNTP_CTL
.endm

.macro CHECK_MONITOR_STACK
  push {r0-r1}     // push to monitor stack

  /* Check monitor stack */
  ldr r0, =monitor_stack_top
  ldr r0, [r0]
  sub r0, r0, #8   // 2 Arm registers
  mov r1, sp
  cmp r0, r1
  bne . // probably a re-entry
.endm

.global handle_timer
handle_timer:
  /* lr points to instruction *after* the interrupted
     instruction. We want to return to what was interrupted.
  */
  sub lr, lr, #4

  /* Do a little dance to copy IRQ mode lr into SVC mode's lr
     See: http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13552.html
  */
  srsdb sp!, #SUPERVISOR_MODE
  cps #SUPERVISOR_MODE
  pop {lr}         // Want IRQ mode LR
  add sp, sp, #4   // Don't want IRQ mode sp

  CHECK_MONITOR_STACK

  /* Next thread is always the scheduler on interrupt */
  ldr r0, =scheduler_thread
  ldr r1, =next_thread
  str r0, [r1]

  DISABLE_TIMER

  b __thread_switch

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
  svc 0xdead
  bx lr

.global handle_svc
handle_svc:
  CHECK_MONITOR_STACK

monitor_stack_ok:
  /* See if this is semihosting or a thread switch */
  ldr r0, [lr, #-4]        // load svc instruction used
  ldr r1, =0xFFFFFF        // mask to get code
  and r0, r0, r1           //
  ldr r1, =0x00dead        // thread switch
  cmp r0, r1
  beq __thread_switch
  ldr r1, =0x123456        // semihosting call
  cmp r0, r1
  beq semihosting
  mov r1, #1               // enable timer
  cmp r0, r1
  beq enable_timer
  mov r1, #0               // disable timer
  cmp r0, r1
  beq disable_timer
  /* Unkown svc code */
  b .

enable_timer:
  ldr r0, =1000
  mcr p15, 0, r0, c14, c2, 0 // CNTP_TVAL
  mov r0, #1                 // Enable, don't mask interrupt
  mcr p15, 0, r0, c14, c2, 1 // CNTP_CTL
  b finalise_timer

disable_timer:
  DISABLE_TIMER
  b finalise_timer

finalise_timer:
  pop {r0-r1}
  srsdb sp!, #SYSTEM_MODE // save CPSR and lr
  cps #SYSTEM_MODE
  b exc_return

semihosting:
  pop {r0-r1}
  cps #SYSTEM_MODE // use user sp to get args
  svc 0x123456     // picked up by Qemu
  srsdb sp!, #SYSTEM_MODE // save CPSR and lr
  b exc_return

__thread_switch:
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

  ldr r1, [r10]           // get actual adress of current thread
  str sp, [r1], #4        // save current stack pointer

  /* update state */
  ldr r2, [r1]
  mov r3, #RUNNING
  cmp r2, r3              // if we're something other than running, leave it as it is
  bne dont_set_state
  mov r3, #SUSPENDED      // otherwise move to suspended
  str r3, [r1]
dont_set_state:

  /* Switch to new thread */
  ldr r11, [r11]          // chase to get actual address of new thread
  str r11, [r10]          // current = to (no need to chase ptr)
  ldr sp, [r11], #4       // restore stack pointer of new thread

  /* check that this thread has been run at least once */
  ldr r3, [r11]           // get state of new thread
  mov r4, #INIT
  cmp r3, r4

  mov r4, #RUNNING        // either way it'll start running
  str r4, [r11]

  bne restore_regs

  /* if not we need to fake the lr and CPSR being on the stack */
  ldr r4, =thread_start
  str r4, [sp, #-4]!      // initialise link register
  mov r5, #USER_MODE      // not sure of mode but flags should be 0
  stmdb sp!, {r4,r5}
  b exc_return            // don't restore other regs, just these two when we return

restore_regs:
  pop {r0-r12, r14}       // restore our own regs (no sp/pc)

exc_return:
  rfeia sp!               // restore CPSR and lr then eret
