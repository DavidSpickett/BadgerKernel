#include "thread_state.h"

.set SYSTEM_MODE,     0x1f
.set SUPERVISOR_MODE, 0x13
.set USER_MODE,       0x10

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r0, r1
  beq \handler
.endm

.macro CURRENT_IN_R10_NEXT_IN_R11
  // Deliberatley callee saved registers so we can call do_scheduler easily
  ldr r10, =_current_thread
  ldr r11, =next_thread
.endm

.macro DISABLE_TIMER
  mov r0, #2                 // Disable timer and mask interrupt
  mcr p15, 0, r0, c14, c2, 1 // CNTP_CTL
.endm

.macro SAVE_KERNEL_TEMPS
  push {r0-r4, r12, lr}
.endm

.macro POP_KERNEL_TEMPS
  pop {r0-r4, r12, lr}
.endm

.macro POP_TEMPS_GOTO_SYSTEM_MODE
  POP_KERNEL_TEMPS
  srsdb sp!, #SYSTEM_MODE // save CPSR and lr to system mode stack pointer
  cps #SYSTEM_MODE        // continue in system mode
.endm

.macro POP_KERNEL_TEMPS_USE_SYSTEM_SP
  POP_KERNEL_TEMPS
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

  SAVE_KERNEL_TEMPS
  DISABLE_TIMER

  /* Next thread is always NULL so that we run the scheduler */
  mov r0, #0
  ldr r1, =next_thread // r1 = &next_thread
  str r0, [r1]         // next_thread = NULL

  b start_thread_switch

.global thread_switch
thread_switch:
  svc svc_thread_switch
  bx lr

.global handle_exception
handle_exception:
  SAVE_KERNEL_TEMPS

  ldr r0, [lr, #-4]        // load svc instruction used
  ldr r1, =0xFFFFFF        // mask to get code
  and r0, r0, r1

  CHECK_SVC svc_thread_switch, start_thread_switch
  CHECK_SVC svc_semihosting, semihosting
  CHECK_SVC svc_disable_timer, disable_timer
  CHECK_SVC svc_enable_timer, enable_timer
  CHECK_SVC svc_syscall, generic_syscall

  b . //unknown svc

generic_syscall:
  // r7 = operation number
  ldr r1, =syscall_handlers
  ldr r7, [r1, r7, LSL#2] // Shift 2 = *4, 4 bytes per address

  // Get back argument registers
  POP_KERNEL_TEMPS
  SAVE_KERNEL_TEMPS

  blx r7      // Call handler
  mov r7, r0  // Temporarily save result

  // If next_thread is NULL by now, just return normally to same thread
  mov r0, #0
  ldr r1, =next_thread
  ldr r1, [r1]
  cmp r0, r1
  beq syscall_ret

  // Otherwise the syscall requires a thread switch
  POP_KERNEL_TEMPS
  mov r0, r7 // Patch in result for eventual resume
  SAVE_KERNEL_TEMPS // So that start_thread_switch can unstack them again
  b start_thread_switch

syscall_ret:
  POP_TEMPS_GOTO_SYSTEM_MODE
  mov r0, r7 // Put result in the right place so that thread will find it when it resumes

  b exception_return

syscall_handlers:
#include "syscall.h"

.global start_thread_switch
start_thread_switch:
  /* If no thread has run before then there's nothing to restore. */
  ldr r0, =_current_thread       // r0 = &_current_thread
  ldr r0, [r0]                   // r0 = _current_thread
  mov r1, #0                     // NULL means we just started up
  cmp r0, r1
  bne check_stack_extent         // If current is valid, save its context
  POP_KERNEL_TEMPS_USE_SYSTEM_SP // Just switch sp, no lr/pc to save to return stack
  CURRENT_IN_R10_NEXT_IN_R11
  b load_next_thread             // Otherwise skip to loading the next thread

check_stack_extent:
  ldr r0, =thread_stack_offset // r0 = &thread_stack_offset
  ldr r1, =_current_thread     // r0 = &_current_thread
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

  CURRENT_IN_R10_NEXT_IN_R11

  /* Save stack pointer */
  ldr r1, [r10]        // r1 = _current_thread
  str sp, [r1], #4     // _current_thread->stack_ptr=sp

  /* Update state */
  ldr r2, [r1]         // r2 = _current_thread->state
  mov r3, #running
  cmp r2, r3           // if we're something other than running, leave it as it is
  bne load_next_thread
  mov r3, #suspended   // otherwise move to suspended
  str r3, [r1]         // _current_thread->state = suspended

load_next_thread:
  // TODO: do_scheduler would ideally not be called here at all
  // For now, check if we're null. If it's not then a syscall must have set it
  // So don't run the scheduler again. (which causes double swaps of loaded programs)
  ldr r0, [r11]
  mov r1, #0
  cmp r0, r1
  ldr r4, [r11]            // Value of next_thread for actually_load_thread to use if we don't branch
  bne actually_load_thread
  cps #SUPERVISOR_MODE     // Use kernel's stack
  bl do_scheduler          // This will set next_thread
  cps #SYSTEM_MODE         // Back to user stack
  ldr r4, [r11]            // Get new next_thread

actually_load_thread:
  str r4, [r10]        // _current_thread = next_thread
  mov r5, #0           // Set next to null for next switch to call scheduler
  str r5, [r11]        // next_thread = NULL
  ldr r10, [r10]       // r10 = _current_thread
  ldr sp, [r10], #4    // sp = current_thread->stack_ptr

  /* check that this thread has been run at least once */
  ldr r3, [r10]        // r3 = _current_thread->state
  mov r4, #init
  cmp r3, r4

  mov r4, #running      // either way it'll start running
  str r4, [r10]         // _current_thread->state = running

  /* Don't restore if it's never run before */
  bne restore_regs

  /* if not we need to fake the lr and CPSR being on the stack */
  ldr r4, =thread_start // init link register
  mov r5, #USER_MODE    // not sure of mode but flags should be 0
  stmdb sp!, {r4,r5}
  b exception_return    // don't restore other regs, just these two when we return

restore_regs:
  pop {r0-r12, r14}     // restore our own regs (no sp/pc)
  b exception_return

semihosting:
  POP_TEMPS_GOTO_SYSTEM_MODE // go to user and set up for return to user mode
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
