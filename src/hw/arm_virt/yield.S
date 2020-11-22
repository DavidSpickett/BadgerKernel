#include "common/svc_calls.h"

.set SYSTEM_MODE,     0x1f
.set SUPERVISOR_MODE, 0x13

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r0, r1
  beq \handler
.endm

.macro CURRENT_IN_R10_NEXT_IN_R11
  // Deliberatley callee saved registers so we can call do_scheduler easily
  ldr r10, =current_thread
  ldr r11, =next_thread
.endm

.macro NEXT_EQUAL_CURRENT
  ldr r0, =next_thread     // Otherwise make next the current thread
  ldr r1, =current_thread // to reload and return to it
  ldr r1, [r1]
  str r1, [r0]             // next_thread = current_thread
.endm

.macro SAVE_CURRENT_THREAD
  srsdb sp!, #SYSTEM_MODE // save CPSR and lr to system mode stack pointer
  cps #SYSTEM_MODE        // continue in system mode

  /* Push all regs apart from sp and pc, but including lr */
  push {r0-r12, r14}

  CURRENT_IN_R10_NEXT_IN_R11

  /* Save stack pointer */
  ldr r1, [r10]        // r1 = current_thread
  str sp, [r1], #4     // current_thread->stack_ptr=sp
.endm

.macro DISABLE_TIMER
  mov r0, #2                 // Disable timer and mask interrupt
  mcr p15, 0, r0, c14, c2, 1 // CNTP_CTL
.endm

.macro CALL_KERNEL function
 cps SUPERVISOR_MODE
 bl \function
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

  SAVE_CURRENT_THREAD
  DISABLE_TIMER

  /* Next thread is always NULL so that we run the scheduler */
  mov r0, #0
  ldr r1, =next_thread // r1 = &next_thread
  str r0, [r1]         // next_thread = NULL

  b load_next_thread

.global handle_exception
handle_exception:
  SAVE_CURRENT_THREAD

  cps SYSTEM_MODE
  ldr r0, [sp, #56] // Get exception pc
  sub r0, r0, #4    // Exception mode lr/pc points to instr after
  ldr r0, [r0]      // Get the svc instr
  ldr r1, =0xFFFFFF // mask to get code
  and r0, r0, r1

  CHECK_SVC svc_thread_switch, load_next_thread
  CHECK_SVC svc_disable_timer, disable_timer
  CHECK_SVC svc_enable_timer, enable_timer
  CHECK_SVC svc_syscall, generic_syscall
  b . // unknown svc

generic_syscall:
  CALL_KERNEL k_handle_syscall
  b load_next_thread

enable_timer:
  ldr r0, =1000
  mcr p15, 0, r0, c14, c2, 0 // CNTP_TVAL
  mov r0, #1                 // Enable, don't mask interrupt
  mcr p15, 0, r0, c14, c2, 1 // CNTP_CTL
  b finalise_timer
disable_timer:
  DISABLE_TIMER
finalise_timer:
  NEXT_EQUAL_CURRENT
  b load_next_thread

.global load_first_thread
load_first_thread:
  // This is called from kernel mode so current_thread is NULL
  // No saving temps here since we came from the kernel
  b load_next_thread

.global load_next_thread
load_next_thread:
  CURRENT_IN_R10_NEXT_IN_R11

  // If next_thread is null then it wasn't set by a syscall
  // Run the scheduler to choose the next thread
  ldr r0, [r11]
  mov r1, #0
  cmp r0, r1
  ldr r4, [r11]            // r4 = *(&next_thread)
  bne actually_load_thread
  // Note that r10/r11 are callee saved so we can still use them after this
  CALL_KERNEL do_scheduler // This will set next_thread
  ldr r4, [r11]            // Get new next_thread

actually_load_thread:
  cps #SYSTEM_MODE     // Back to user stack
  str r4, [r10]        // current_thread = next_thread
  mov r5, #0           // Set next to null for next switch to call scheduler
  str r5, [r11]        // next_thread = NULL
  ldr r10, [r10]       // r10 = current_thread
  ldr sp, [r10]        // sp = current_thread->stack_ptr

  /* even an init state thread will have a blank context to restore */
  pop {r0-r12, r14}     // restore our own regs (no sp/pc)

  rfeia sp! // restore CPSR and lr then eret

.global signal_handler_wrapper
signal_handler_wrapper:
  // r0 = signal, r1 = handler
  blx r1
  svc svc_thread_switch
.global signal_handler_wrapper_end
signal_handler_wrapper_end:
  nop

.global get_context
get_context:
  // r0 = destination pointer
  str r4,  [r0], #4
  str r5,  [r0], #4
  str r6,  [r0], #4
  str r7,  [r0], #4
  str r8,  [r0], #4
  str r9,  [r0], #4
  str r10, [r0], #4
  str r11, [r0], #4
  str sp,  [r0], #4
  str lr,  [r0], #4
  str lr,  [r0], #4 // Default new PC to immediatley after get_context call
  bx lr

.global set_context
set_context:
  // r0 = source pointer
  ldr r4,  [r0], #4
  ldr r5,  [r0], #4
  ldr r6,  [r0], #4
  ldr r7,  [r0], #4
  ldr r8,  [r0], #4
  ldr r9,  [r0], #4
  ldr r10, [r0], #4
  ldr r11, [r0], #4
  ldr sp,  [r0], #4
  ldr lr,  [r0], #4
  ldr r1,  [r0], #4 // Our new PC
  bx r1

// Used to catch fibres returning normally
.global set_context_from_stack_address
set_context_from_stack_address:
  ldr r0, [sp], #4
  b set_context

.macro SWAP reg
  ldr r1,   [r0]
  str \reg, [r0], #4
  mov \reg, r1
.endm

.global swap_context
swap_context:
  // r0 = location of ctx to swap into/save to
  SWAP r4
  SWAP r5
  SWAP r6
  SWAP r7
  SWAP r8
  SWAP r9
  SWAP r10
  SWAP r11
  SWAP sp
  ldr r1, [r0]     // Load new lr
  str lr, [r0], #4 // Store current lr
  ldr r2, [r0]     // Load new pc
  str lr, [r0], #4 // Store pc (the lr of this fn)
  mov lr, r1       // Set new lr
  mov r1, r2       // New pc to branch to
  sub r0, #11*4    // Reset r0 to be arg1 of the new function
  bx r1
