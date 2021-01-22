#include "common/svc_calls.h"

@ This allows us to use push/pop with high registers
.syntax unified

.set ICSR, 0xE000ED04
.set NVIC_ICER0, 0XE000E180

.macro CHECK_SVC code, handler
  mov r1, #\code
  cmp r2, r1
  beq \handler
.endm

.macro CURRENT_IN_R6_NEXT_IN_R7
  @ Callee saved regs so we can call do_scheduler
  ldr r6, =current_thread
  ldr r7, =next_thread
.endm

.macro NEXT_EQUAL_CURRENT
  CURRENT_IN_R6_NEXT_IN_R7
  ldr r6, [r6]
  str r6, [r7]
.endm

.macro SAVE_CURRENT_THREAD
  @ Set MSP = PSP
  @ We need to do this ourselves because writes to CONTROL.SPSEL
  @ are ignored in handler mode.
  mrs r0, psp
  mov sp, r0

  @ callee saved regs
  push {r4-r11} @ no lr, it's already on the stack

  CURRENT_IN_R6_NEXT_IN_R7

  @ Save stack pointer
  ldr r1, [r6]           @ r1 = current_thread
  str sp, [r1], #4       @ current_thread->stack_ptr = sp
.endm

@ This is just the load, returning into it can be handled differently
.macro LOAD_NEXT_THREAD
  CURRENT_IN_R6_NEXT_IN_R7
  ldr r4, [r7]          @ r4 = next_thread
  str r4, [r6]          @ current_thread = next_thread
  mov r4, #0            @ Set next to null for next switch to call scheduler
  str r4, [r7]          @ next_thread = NULL
  ldr r6, [r6]          @ r6 = current_thread
  ldr r5, [r6]          @ r5 = current_thread->stack_ptr
  mov sp, r5            @ MSP = new thread's stack pointer

  @ No need to restore PC, exc return will do that

  @ Even init threads have some saved context setup by init_thread
  pop {r4-r11}
.endm

.macro CALL_KERNEL function
  ldr r5, =stack_top @ Use kernel stack
  mov sp, r5
  push {lr}
  bl \function
  pop {lr}
.endm

.global handle_exception
.thumb_func
handle_exception:
  SAVE_CURRENT_THREAD

  @ Check the exception/interrupt number first so we
  @ don't misdiagnose an instruction ending in FF or AB
  @ as an SVC.
  ldr r0, =ICSR
  ldr r0, [r0]
  mov r1, #0xff    @ VECTACTIVE is the bottom 8 bits
  and r0, r0, r1
  mov r1, #15      @ Timer int
  cmp r0, r1
  beq handle_timer
  mov r1, #11      @ SVC
  cmp r0, r1
  beq handle_svc
  @ something unknown
  b .

handle_timer:
  @ Disable interrupts so the scheduler pick next thread
  ldr r0, =NVIC_ICER0
  mvn r1, #0     @ aka 0xFFFFFFFF, write ones to disable all
  str r1, [r0]

  @ Next thread is always null to run the scheduler
  mov r0, #0
  str r0, [r1]         @ next_thread = NULL

  b load_next_thread
  @ Pending status is cleared by exception return

handle_svc:
  ldr r2, [sp, #14*4] @ find the PC we came from
  sub r2, r2, #2      @ back one instruction to the svc
  ldr r2, [r2]        @ load svc instruction
  mov r1, #0xff       @ mask out the svc number
  and r2, r1, r2

  CHECK_SVC svc_thread_switch, load_next_thread
  CHECK_SVC svc_syscall,       generic_syscall
  CHECK_SVC svc_disable_timer, disable_timer
  CHECK_SVC svc_enable_timer, enable_timer
  b . @ unknown svc

generic_syscall:
  CALL_KERNEL k_handle_syscall
  b load_next_thread

enable_timer:
  CALL_KERNEL k_enable_timer
  b finalise_timer
disable_timer:
  CALL_KERNEL k_disable_timer
finalise_timer:
  NEXT_EQUAL_CURRENT
  b load_next_thread

.global load_first_thread
.thumb_func
load_first_thread:
  @ Unable to exception return to thread mode after a reset,
  @ no matter what EXC_RETURN is. *shrug*
  @ Second best option is to load up first thread manually.
  bl do_scheduler          @ Trashes lr but whatever

  mrs r0, control
  mov r1, #3               @ Unprivleged, use PSP
  orr r0, r0, r1
  msr control, r0
  isb

  CURRENT_IN_R6_NEXT_IN_R7
  LOAD_NEXT_THREAD

  ldr lr, =thread_start
  bx lr

.thumb_func
load_next_thread:
  @ When a stack check for the current thread fails,
  @ this is called from a kernel function. Meaning
  @ that the magic lr value has been trashed.
  @ So always reload to cover that case.
  mov lr, #0xFFFFFFFD  @ Special return to user lr value
  ldr r0, =stack_top       @ Back to kernel stack
  mov sp, r0

  CURRENT_IN_R6_NEXT_IN_R7
  @ If next thread is null then it wasn't set by a syscall
  @ Run the scheduler to choose the next thread
  ldr r4, [r7] @ r4 = *(&next_thread)
  mov r1, #0
  cmp r4, r1
  bne actually_load_thread

  @ Otherwise choose a new next_thread and do housekeping
  @ Note that r6/r7 are callee saved so we can still use them after this
  push {lr}
  bl do_scheduler          @ This will set next_thread
  pop {lr}
  ldr r4, [r7]             @ Get new next_thread

actually_load_thread:
  LOAD_NEXT_THREAD

  @ Set PSP after we have unstacked everything so it's
  @ correct for automatic exception return.
  mov r0, sp @ r0 ok to trash because it's in the exception frame
  msr psp, r0
  bx lr

.global signal_handler_wrapper
.thumb_func
signal_handler_wrapper:
  @ r0 = signal, r1 = handler
  blx r1
  svc svc_thread_switch
.global signal_handler_wrapper_end
signal_handler_wrapper_end:
  nop

.global get_context
.thumb_func
get_context:
  @ r0 = destination pointer
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
  @ Default new PC to immediatley after get_context call
  str lr,  [r0], #4
  bx lr

.global set_context
.thumb_func
set_context:
  @ r0 = source pointer
  ldr r4,  [r0], #4
  ldr r5,  [r0], #4
  ldr r6,  [r0], #4
  ldr r7,  [r0], #4
  ldr r8,  [r0], #4
  ldr r9,  [r0], #4
  ldr r10, [r0], #4
  ldr r11, [r0], #4
  @ Loading straight into sp can be unsafe with interrupts on
  ldr r1,  [r0], #4
  mov sp, r1
  ldr lr,  [r0], #4
  ldr r1,  [r0], #4 @ Our new PC
  bx r1

@ Used to catch fibers returning normally
.global set_context_from_stack_address
.thumb_func
set_context_from_stack_address:
  ldr r0, [sp], #4
  b set_context

.macro SWAP reg
  ldr r1,   [r0]
  str \reg, [r0], #4
  mov \reg, r1
.endm

.global swap_context
.thumb_func
swap_context:
  @ r0 = location of ctx to swap into/save to
  SWAP r4
  SWAP r5
  SWAP r6
  SWAP r7
  SWAP r8
  SWAP r9
  SWAP r10
  SWAP r11
  SWAP sp
  ldr r1, [r0]     @ Load new lr
  str lr, [r0], #4 @ Store current lr
  ldr r2, [r0]     @ Load new pc
  str lr, [r0], #4 @ Store pc (the lr of this fn)
  mov lr, r1       @ Set new lr
  mov r1, r2       @ New pc to branch to
  sub r0, #11*4    @ Reset r0 to be arg1 of the new function
  bx r1
