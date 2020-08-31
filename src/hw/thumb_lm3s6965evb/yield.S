#include "common/thread_state.h"

// This allows us to use push/pop with high registers
.syntax unified

.set ICSR, 0xE000ED04
.set NVIC_ICER0, 0XE000E180

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r2, r1
  beq \handler
.endm

.macro CURRENT_IN_R6_NEXT_IN_R7
  // Callee saved regs so we can call do_scheduler
  ldr r6, =_current_thread
  ldr r7, =next_thread
.endm

.macro NEXT_EQUAL_CURRENT
  CURRENT_IN_R6_NEXT_IN_R7
  ldr r6, [r6]
  str r6, [r7]
.endm

.macro SAVE_CURRENT_THREAD
  /* Set MSP = PSP
     We need to do this ourselves because writes to CONTROL.SPSEL
     are ignored in handler mode. */
  mrs r0, psp
  mov sp, r0

  /* callee saved regs */
  push {r4-r11} // no lr, it's already on the stack

  CURRENT_IN_R6_NEXT_IN_R7

  /* Save stack pointer */
  ldr r1, [r6]           // r1 = _current_thread
  str sp, [r1], #4       // _current_thread->stack_ptr = sp

  /* Update state */
  ldr r2, [r1]          // r2 = _current_thread->state
  mov r3, #running
  cmp r2, r3            // if we're something other than running, leave it as it is
  bne 1f
  mov r3, #suspended    // otherwise move to suspended
  str r3, [r1]          // _current_thread->state = suspended
1:
.endm

// This is just the load, returning into it can be handled differently
.macro LOAD_NEXT_THREAD
  CURRENT_IN_R6_NEXT_IN_R7
  ldr r4, [r7]          // r4 = next_thread
  str r4, [r6]          // _current_thread = next_thread
  mov r4, #0            // Set next to null for next switch to call scheduler
  str r4, [r7]          // next_thread = NULL
  ldr r6, [r6]          // r6 = _current_thread
  ldr r5, [r6]          // r5 = _current_thread->stack_ptr
  mov sp, r5            // MSP = new thread's stack pointer

  /* no need to restore PC, exc return will do that */

  mov r4, #running      // start running
  add r6, r6, #4        // r6 = &(current_thread->state)
  str r4, [r6]          // _current_thread->state = running

  // Even init threads have some saved context setup by init_thread
  pop {r4-r11}
.endm

.global thread_switch_from_kernel_mode
thread_switch_from_kernel_mode:
  mov lr, #0xFFFFFFFD  // Special return to user lr value
  b load_next_thread

.global handle_exception
.thumb_func
handle_exception:
  SAVE_CURRENT_THREAD

  /* check the exception/interrupt number first so we
     don't misdiagnose an instruction ending in FF or AB
     as an SVC. */
  ldr r0, =ICSR
  ldr r0, [r0]
  mov r1, #0xff    // VECTACTIVE is the bottom 8 bits
  and r0, r0, r1
  mov r1, #15      // Timer int
  cmp r0, r1
  beq handle_timer
  mov r1, #11      // SVC
  cmp r0, r1
  beq handle_svc
  /* something unknown */
  b .

handle_timer:
  /* Disable interrupts so the scheduler pick next thread */
  ldr r0, =NVIC_ICER0
  mvn r1, #0     // aka 0xFFFFFFFF, write ones to disable all
  str r1, [r0]

  /* Next thread is always null to run the scheduler */
  mov r0, #0
  str r0, [r1]         // next_thread = NULL

  b load_next_thread
  /* Pending status is cleared by exception return */

handle_svc:
  ldr r2, [sp, #14*4] // find the PC we came from
  sub r2, r2, #2  // back one instruction to the svc
  ldr r2, [r2]    // load svc instruction
  mov r1, #0xff   // mask out the svc number
  and r2, r1, r2

  CHECK_SVC svc_thread_switch, load_next_thread
  CHECK_SVC svc_syscall,       generic_syscall
  // no disable_timer - done from user mode
  // no enable_timer - done from user mode
  b . // unknown svc

generic_syscall:
  // Get back arguments and operation number
  ldr r0, [sp, #8*4]
  ldr r1, [sp, #9*4]
  ldr r2, [sp, #10*4]
  ldr r3, [sp, #11*4]
  ldr r8, [sp, #4*4]

  mov r6, sp        // Save current sp for after call (callee saved)
  ldr r5, =stack_top // Use kernel stack
  mov sp, r5

  // r8 = operation number
  ldr r5, =syscall_handlers
  ldr r8, [r5, r8, LSL#2] // Shift 2 = *4, 4 bytes per address

  push {lr}
  blx r8              // call handler
  pop {lr}
  str r0, [r6, #8*4] // Store result back in context

  // If next_thread is NULL set it to current to resume current thread
  mov r0, #0
  ldr r1, =next_thread
  ldr r1, [r1]
  cmp r0, r1
  bne 1f
  NEXT_EQUAL_CURRENT
1:
  b load_next_thread

syscall_handlers:
#include "common/syscall.h"

.global load_first_thread
.thumb_func
load_first_thread:
  /* Unable to exception return to thread mode after a reset,
     no matter what EXC_RETURN is. *shrug*
     Second best option is to load up first thread manually. */
  bl do_scheduler          // Trashes lr but whatever

  mrs r0, control
  mov r1, #3               // Unprivleged, use PSP
  orr r0, r0, r1
  msr control, r0
  isb

  CURRENT_IN_R6_NEXT_IN_R7
  LOAD_NEXT_THREAD

  ldr lr, =thread_start
  bx lr

load_next_thread:
  ldr r0, =stack_top       // Back to kernel stack
  mov sp, r0

  CURRENT_IN_R6_NEXT_IN_R7
  // If next thread is null then it wasn't set by a syscall
  // Run the scheduler to choose the next thread
  ldr r4, [r7] // r4 = *(&next_thread)
  mov r1, #0
  cmp r4, r1
  bne actually_load_thread

  // Otherwise choose a new next_thread and do housekeping
  // Note that r6/r7 are callee saved so we can still use them after this
  push {lr}
  bl do_scheduler          // This will set next_thread
  pop {lr}
  ldr r4, [r7]             // Get new next_thread

actually_load_thread:
  LOAD_NEXT_THREAD

  /* Set PSP after we have unstacked everything so it's
     correct for automatic exception return. */
  mov r0, sp // r0 ok to trash because it's in the exception frame
  msr psp, r0
  bx lr
