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

  b check_stack_extent

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

  CHECK_SVC svc_thread_switch, check_stack_extent
  CHECK_SVC svc_disable_timer, disable_timer
  CHECK_SVC svc_enable_timer, enable_timer
  CHECK_SVC svc_syscall, generic_syscall
  b . // unknown svc

generic_syscall:
  // r8 = operation number
  ldr r1, =syscall_handlers
  ldr r8, [r1, r8, LSL#2] // Shift 2 = *4, 4 bytes per address

  // Get back argument registers
  POP_KERNEL_TEMPS
  SAVE_KERNEL_TEMPS

  blx r8      // Call handler
  mov r8, r0  // Temporarily save result

  // If next_thread is NULL by now, just return normally to same thread
  mov r0, #0
  ldr r1, =next_thread
  ldr r1, [r1]
  cmp r0, r1
  beq syscall_ret

  // Otherwise the syscall requires a thread switch
  POP_KERNEL_TEMPS
  mov r0, r8 // Patch in result for eventual resume
  SAVE_KERNEL_TEMPS // So that they can get unstacked later
  b check_stack_extent

syscall_ret:
  POP_TEMPS_GOTO_SYSTEM_MODE
  mov r0, r8 // Put result in the right place so that thread will find it when it resumes
  b exception_return

syscall_handlers:
#include "common/syscall.h"

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

.global load_first_thread
load_first_thread:
  // This is called from kernel mode so _current_thread is NULL
  // No saving temps here since we came from the kernel
  CURRENT_IN_R10_NEXT_IN_R11
  b load_next_thread           // load first thread

check_stack_extent:
  // TODO: this is only checking *overflow*
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
  // If next_thread is null then it wasn't set by a syscall
  // Run the scheduler to choose the next thread
  ldr r0, [r11]
  mov r1, #0
  cmp r0, r1
  ldr r4, [r11]            // r4 = *(&next_thread)
  bne actually_load_thread
  cps #SUPERVISOR_MODE     // Use kernel's stack
  // Note that r10/r11 are callee saved so we can still use them after this
  bl do_scheduler          // This will set next_thread
  ldr r4, [r11]            // Get new next_thread

actually_load_thread:
  cps #SYSTEM_MODE     // Back to user stack
  str r4, [r10]        // _current_thread = next_thread
  mov r5, #0           // Set next to null for next switch to call scheduler
  str r5, [r11]        // next_thread = NULL
  ldr r10, [r10]       // r10 = _current_thread
  ldr sp, [r10], #4    // sp = _current_thread->stack_ptr

  mov r4, #running      // start running
  str r4, [r10]         // _current_thread->state = running

  /* even an init state thread will have a blank context to restore */
  pop {r0-r12, r14}     // restore our own regs (no sp/pc)
  b exception_return

exception_return:
  rfeia sp! // restore CPSR and lr then eret
