#include "thread_state.h"

.set SYSTEM_MODE,     0x1f
.set SUPERVISOR_MODE, 0x13
.set USER_MODE,       0x10

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r0, r1
  beq \handler
.endm

.macro CURRENT_IN_R6_NEXT_IN_R7
  ldr r6, =_current_thread
  ldr r7, =next_thread
.endm

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
  bne .                    // probably a re-entry
.endm

.macro POP_TEMPS_GOTO_SYSTEM_MODE
  pop {r0, r1}            // pop temps from supervisor stack
  srsdb sp!, #SYSTEM_MODE // save CPSR and lr to system mode stack pointer
  cps #SYSTEM_MODE        // continue in system mode
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
  DISABLE_TIMER

  /* Next thread is always the scheduler on interrupt */
  ldr r0, =scheduler_thread
  ldr r1, =next_thread
  str r0, [r1]         // next_thread = &scheduler_thread

  b start_thread_switch

.global thread_switch
thread_switch:
  svc svc_thread_switch
  bx lr

.global handle_exception
handle_exception:
  CHECK_MONITOR_STACK

  /* See if this is semihosting or a thread switch */
  ldr r0, [lr, #-4]        // load svc instruction used
  ldr r1, =0xFFFFFF        // mask to get code
  and r0, r0, r1
  CHECK_SVC svc_thread_switch, start_thread_switch
  CHECK_SVC svc_semihosting, semihosting
  CHECK_SVC svc_enable_timer, enable_timer
  CHECK_SVC svc_disable_timer, disable_timer
  /* unknown svc */
  b .

start_thread_switch:
  /* If the current thread has never run, don't save anything.
     Used for initial scheduler start and recovering from stack
     boundary issues. */
  ldr r0, =_current_thread // r0 = &_current_thread
  ldr r0, [r0]             // r0 = _current_thread
  ldr r0, [r0, #4]         // r0 = _current_thread->state
  mov r1, #init
  cmp r0, r1
  bne check_stack_extent
  POP_TEMPS_GOTO_SYSTEM_MODE
  CURRENT_IN_R6_NEXT_IN_R7
  b load_next_thread

check_stack_extent:
  ldr r0, =thread_stack_offset
  ldr r1, =_current_thread
  ldr r0, [r0]                 // r0 = thread_stack_offset
  ldr r1, [r1]                 // r1 = _current_thread
  add r0, r1, r0               // get minimum valid stack pointer
  cps #SYSTEM_MODE
  mov r1, sp                   // get thread's stack pointer
  cps #SUPERVISOR_MODE
  sub r1, r1, #((13+1+1+1)*4)  // 13 gp regs plus lr plus lr+CPSR
  cmp r0, r1                   // is potential sp < min valid sp?
  bhs stack_extent_failed      // call C function to error and exit

save_current_thread:
  POP_TEMPS_GOTO_SYSTEM_MODE

  /* Push all regs apart from sp and pc, but including lr */
  push {r0-r12, r14}

  CURRENT_IN_R6_NEXT_IN_R7

  /* Save stack pointer */
  ldr r1, [r6]         // r1 = _current_thread
  str sp, [r1], #4     // _current_thread->stack_ptr=sp

  /* Update state */
  ldr r2, [r1]         // r2 = _current_thread->state
  mov r3, #running
  cmp r2, r3           // if we're something other than running, leave it as it is
  bne load_next_thread
  mov r3, #suspended   // otherwise move to suspended
  str r3, [r1]         // _current_thread->state = suspended

load_next_thread:
  ldr r7, [r7]         // r11 = next_thread
  str r7, [r6]         // _current_thread = next_thread
  ldr sp, [r7], #4     // sp = next_thread->stack_ptr

  /* check that this thread has been run at least once */
  ldr r3, [r7]          // r3 = next_thread->state
  mov r4, #init
  cmp r3, r4

  mov r4, #running      // either way it'll start running
  str r4, [r7]          // next_thread->state = running

  /* Don't restore if it's never run before */
  bne restore_regs

  /* if not we need to fake the lr and CPSR being on the stack */
  ldr r4, =thread_start // init link register
  mov r5, #USER_MODE    // not sure of mode but flags should be 0
  stmdb sp!, {r4,r5}
  b exception_return          // don't restore other regs, just these two when we return

restore_regs:
  pop {r0-r12, r14}     // restore our own regs (no sp/pc)
  b exception_return

semihosting:
  // use user r0/r1 and sp to get arguments
  POP_TEMPS_GOTO_SYSTEM_MODE
  svc 0x123456     // picked up by Qemu
  b exception_return

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
  POP_TEMPS_GOTO_SYSTEM_MODE
  b exception_return

exception_return:
  rfeia sp! // restore CPSR and lr then eret
