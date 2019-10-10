#include "thread_state.h"

// This allows us to use push/pop with high registers
.syntax unified

.set ICSR, 0xE000ED04
.set NVIC_ICER0, 0XE000E180

.global thread_switch
.thumb_func
thread_switch:
  svc svc_thread_switch
  bx lr

.global thread_switch_initial
.thumb_func
thread_switch_initial:
  /* Init stack ptr to dummy thread so we can yield to scheduler */
  ldr r0, =_current_thread // Set the actual stack pointer to
  ldr r0, [r0]            // that of the dummy thread
  ldr r0, [r0]            // so that we can pass the stack check
  mov sp, r0              // without having more than one entry point
  bl thread_switch        // to the switching code
  bx lr

.global handle_exception
.thumb_func
handle_exception:
  /* Stack pointer is MSP aka monitor stack here.
     Switch to privileged mode, but use the PSP,
     since we don't need monitor stack.
  */
  mrs r0, control
  mov r1, #1      // privileged mode
  mvn r1, r1
  and r0, r0, r1
  msr control, r0
  isb

  /* r0-3, r12, lr, pc xPSR have already been saved
     on the PSP stack. So we won't be using the monitor
     stack at all and don't need to validate it here.
  */

  /* See if this is a thread switch or a semihosting call */

  /* check the exception/interrupt number first so we
     don't misdiagnose an instruction ending in FF or AB
     as an SVC. */
  ldr r0, =ICSR
  ldr r0, [r0]
  mov r1, #0xff    // VECTACTIVE is the bottom 8 bits
  and r0, r0, r1
  mov r1, #15      // Timer int
  cmp r0, r1
  beq handle_interrupt
  mov r1, #11      // SVC
  cmp r0, r1
  beq handle_svc
  b exc_err

handle_interrupt:
  /* Disable interrupts so the scheduler pick next thread */
  ldr r0, =NVIC_ICER0
  mvn r1, #0     // aka 0xFFFFFFFF, write ones to disable all
  str r1, [r0]

  /* Set next thread to be the scheduler regardless of what
     it currently is. Since we've no idea what we interrupted. */
  ldr r0, =scheduler_thread
  ldr r1, =next_thread   // Don't need any chasing here
  str r0, [r1]

  /* set state to suspended */
  ldr r0, =_current_thread
  ldr r0, [r0]
  add r0, #4         // Skip stack ptr
  mov r1, #suspended

  beq __thread_switch
  /* Pending status is cleared by exception return */

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r0, r1
  beq \handler
.endm

handle_svc:
  mrs r0, psp
  mov r1, #(6*4)
  add r0, r1      // find the PC we came from
  ldr r0, [r0]
  mvn r1, #1      // remove mode bit (not sure if it's always there)
  and r0, r0, r1
  sub r0, r0, #2  // back one instruction to the svc
  ldr r0, [r0]    // load svc instruction
  mov r1, #0xff   // mask out the svc number
  and r0, r1, r0

  CHECK_SVC svc_thread_switch, __thread_switch
  CHECK_SVC svc_semihosting,   semihosting
  /* timer control is done from user mode */

exc_err:
  /* something unexpected */
  b .

semihosting:
  /* We need to use the PSP here and the thread's
     r0/r1 because that will contain the semihosting
     call args */
  mrs r2, psp
  ldr r0, [r2]       // pop r0
  add r2, #4         // no direct post increment in thumb
  ldr r1, [r2]       // pop r1

  bkpt 0xab          // actual semihosting call, handled by Qemu

  sub r2, #4         // save the return value from r0
  str r0, [r2]       // will get restored to thread automatically

  bx lr              // return to thread

__thread_switch:
  /* Check stack extent
     Hardware has already stacked r0-3 for us. So we want
     space to save r4-r7. If the automatic saving also caused
     an underflow we'll detect it too.
  */
  ldr r0, =thread_stack_offset
  ldr r1, =_current_thread
  ldr r0, [r0]                 // chase offset
  ldr r1, [r1]                 // chase current
  add r0, r1, r0               // get min. valid stack pointer
  mrs r1, psp                  // get current sp
  sub r1, r1, #(7*4)           // take away (extra) space we want to use
  cmp r0, r1                   // is potential sp < min valid sp?
  ldr r2, =stack_extent_failed // can't get a relocation to this, use addr
  bls stack_check_passed       // can't conditonally branch to register...
  bx r2                        // so branch over this instr if check passed

stack_check_passed:
  /* carry on with the yield */
  mrs r0, psp
  mov sp, r0   // MSP = PSP

  /* callee saved regs */
  push {r4-r11} // no lr, it's already on the stack

  /* Setup pointers some high reg numbers */
  ldr r6, =_current_thread
  ldr r7, =next_thread

  /* Save stack pointer */
  ldr r1, [r6]           // get actual adress of current thread
  mov r5, sp             // save current stack pointer
  str r5, [r1]
  add r1, r1, #4

  /* Update state */
  ldr r2, [r1]
  mov r3, #running
  cmp r2, r3
  bne dont_update_state
  mov r2, #suspended
  str r2, [r1]
dont_update_state:

  /* Switch to new thread */
  ldr r7, [r7]          // chase to get actual address of next thread
  str r7, [r6]          // _current_thread = next_thread
  ldr r5, [r7]          // restore stack pointer of new thread
  mov sp, r5            // MSP = thread stack pointer
  add r7, #4
  /* no need to restore PC, exc return will do that */

  /* check that thread has been scheduled at least once already */
  ldr r3, [r7]          // load state
  mov r4, #init
  cmp r3, r4

  /* either way it'll start running */
  mov r4, #running
  str r4, [r7]

  bne restore_regs

  /* If it's never been scheduled we need to fake PC and EPSR
     values being on the stack. (lr doesn't matter) */
  sub sp, #(8*4)       // size of the expected return frame
  ldr r4, =thread_start
  str r4, [sp, #(6*4)] // store intial pc
  mov r1, #1           // Set Thumb bit in EPSR
  lsl r1, #24
  str r1, [sp, #(7*4)] // store intial EPSR
  b exc_return

restore_regs:
  pop {r4-r11}

exc_return:
  /* Set the psp *after* we've unstacked r4-r7.
     So it's where the automatic unstack expects it to be.
  */
  mov r0, sp
  msr psp, r0

  /* We don't use the monitor stack but we might as well
     keep it consistent. */
  ldr r0, =monitor_stack_top
  ldr r0, [r0]

  bx lr
