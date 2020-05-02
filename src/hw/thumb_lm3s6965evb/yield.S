#include "thread_state.h"

// This allows us to use push/pop with high registers
.syntax unified

.set ICSR, 0xE000ED04
.set NVIC_ICER0, 0XE000E180

.macro CURRENT_IN_R6_NEXT_IN_R7
  ldr r6, =_current_thread
  ldr r7, =next_thread
.endm

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r0, r1
  beq \handler
.endm

.global thread_switch
.thumb_func
thread_switch:
  svc svc_thread_switch
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
  /* something unknown */
  b .

handle_interrupt:
  /* Disable interrupts so the scheduler pick next thread */
  ldr r0, =NVIC_ICER0
  mvn r1, #0     // aka 0xFFFFFFFF, write ones to disable all
  str r1, [r0]

  /* Next thread is always the scheduler on interrupt */
  ldr r0, =scheduler_thread
  ldr r1, =next_thread
  str r0, [r1]         // next_thread = &scheduler_thread

  beq start_thread_switch
  /* Pending status is cleared by exception return */

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

  CHECK_SVC svc_thread_switch, start_thread_switch
  CHECK_SVC svc_semihosting,   semihosting
  /* timer control is done from user mode */
  /* unknown svc */
  b .

start_thread_switch:
  /* Set MSP = PSP */
  mrs r0, psp
  mov sp, r0

  /* If the current thread has never run, don't save anything.
     Used for initial scheduler start and recovering from stack
     boundary issues. */
  ldr r0, =_current_thread // r0 = &_current_thread
  ldr r0, [r0]             // r0 = _current_thread
  ldr r0, [r0, #4]         // r0 = _current_thread->state
  mov r1, #init
  cmp r0, r1
  bne check_stack_extent
  CURRENT_IN_R6_NEXT_IN_R7
  b load_next_thread

check_stack_extent:
  /* Hardware has already stacked r0-3 for us. So we want
     space to save r4-r7. If the automatic saving also caused
     an underflow we'll detect it too.
  */
  ldr r0, =thread_stack_offset
  ldr r1, =_current_thread
  ldr r0, [r0]                 // r0 = thread_stack_offset
  ldr r1, [r1]                 // r1 = _current_thread
  add r0, r1, r0               // get min. valid stack pointer
  mrs r1, psp                  // get current sp
  sub r1, r1, #(7*4)           // take away (extra) space we want to use
  cmp r0, r1                   // is potential sp < min valid sp?
  ldr r2, =stack_extent_failed // can't get a relocation to this, use addr
  bls save_current_thread      // can't conditonally branch to register...
  bx r2                        // so branch over this instr if check passed

save_current_thread:
  /* callee saved regs */
  push {r4-r11} // no lr, it's already on the stack

  CURRENT_IN_R6_NEXT_IN_R7

  /* Save stack pointer */
  ldr r1, [r6]           // get actual adress of current thread
  mov r5, sp             // save current stack pointer
  str r5, [r1]

  /* Update state */
  add r1, r1, #4
  ldr r2, [r1]          // r2 = _current_thread->state
  mov r3, #running
  cmp r2, r3            // if we're something other than running, leave it as it is
  bne load_next_thread
  mov r3, #suspended    // otherwise move to suspended
  str r3, [r1]          // _current_thread->state = suspended

load_next_thread:
  ldr r7, [r7]          // r11 = next_thread
  str r7, [r6]          // _current_thread = next_thread
  ldr r5, [r7]          // sp = next_thread->stack_ptr
  mov sp, r5            // MSP = thread stack pointer
  /* no need to restore PC, exc return will do that */

  /* check that this thread has been run at least once */
  add r7, #4
  ldr r3, [r7]          // r3 = next_thread->state
  mov r4, #init
  cmp r3, r4

  mov r4, #running      // either way it'll start running
  str r4, [r7]          // next_thread->state = running

  /* Don't restore if it's never run before */
  bne restore_regs

  /* If it's never been scheduled we need to fake PC and EPSR
     values being on the stack. (lr doesn't matter) */
  sub sp, #(8*4)       // size of the expected return frame
  ldr r4, =thread_start
  str r4, [sp, #(6*4)] // store intial pc
  mov r1, #1           // Set Thumb bit in EPSR
  lsl r1, #24
  str r1, [sp, #(7*4)] // store intial EPSR
  b exception_return

restore_regs:
  pop {r4-r11}

exception_return:
  /* Set the psp *after* we've unstacked r4-r7.
     So it's where the automatic unstack expects it to be.
  */
  mov r0, sp
  msr psp, r0

  bx lr

semihosting:
  // use user r0/r1 and sp to get arguments
  mrs r2, psp
  ldr r0, [r2]       // pop r0
  add r2, #4         // no direct post increment in thumb
  ldr r1, [r2]       // pop r1

  bkpt 0xab          // actual semihosting call, handled by Qemu

  sub r2, #4         // save the return value from r0
  str r0, [r2]       // will get restored to thread automatically

  bx lr              // return to thread
