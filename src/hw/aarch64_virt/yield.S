#include "thread_state.h"

.set TIMER_AMOUNT, 100000

.macro DISABLE_TIMER
  mov x0, #2    // Disable timer output, mask the interrupt
  msr CNTV_CTL_EL0, x0
.endm

.macro CHECK_MONITOR_STACK
  msr SPSel, #1
  /* Use the monitor stack, we can't trust the thread stack here */
  stp x0, x1, [sp, #-16]!

  ldr x0, =monitor_stack // check that monitor stack is valid
  mov x1, sp
  cmp x0, x1
  beq 1f                 // monitor stack is valid
  b .                    // probably a re-entry
.endm

.global thread_switch_initial
thread_switch_initial:
  /* Called when starting scheduler (trashing regs is fine) */
  ldr x0, =_current_thread // init stack pointer to
  ldr x0, [x0]            // stack pointer of dummy thread
  ldr x0, [x0]            // so we can pass the check normally
  mov sp, x0
  bl thread_switch

.global thread_switch
thread_switch:
  svc 0xdead
  ret

/* Having this as a seperate handler is easier than
   finding the exact right register to read.
   Since I'm not sure what would happen if there
   were a pending timer int, and we happened to hit
   an SVC at the same time. We might lose the SVC.
*/
.global handle_timer
handle_timer:
  CHECK_MONITOR_STACK
1:
  DISABLE_TIMER

  /* Always set next thread to scheduler */
  ldr x0, =scheduler_thread
  ldr x1, =next_thread
  str x0, [x1]

  b __thread_switch

.macro CHECK_SVC code, handler
  mov x1, #\code
  cmp x0, x1
  beq \handler
.endm

.global handle_svc
handle_svc:
  CHECK_MONITOR_STACK
1:
  /* See what brought us here. */
  mrs x0, ESR_EL1
  lsr x0, x0, #26    // check exception code
  mov x1, #0x15      // SVC
  cmp x0, x1
  beq check_svc

  b unknown_exc

check_svc:
  mrs x0, ESR_EL1    // Reload then check svc code
  mov x1, #0xFFFF    // mask svc number
  and x0, x0, x1
  CHECK_SVC 0xdead, __thread_switch
  CHECK_SVC 0x3333, semihosting
  CHECK_SVC 1,      enable_timer
  CHECK_SVC 0,      disable_timer

unknown_exc:
  /* Otherwise it's something we weren't expecting */
  b .

enable_timer:
  mrs x0, CNTVCT_EL0     // Get current count
  ldr x1, =TIMER_AMOUNT
  add x1, x0, x1
  msr CNTV_CVAL_EL0, x1  // New target is some point in the future

  mov x0, #1
  msr CNTV_CTL_EL0, x0

  b finalise_timer

disable_timer:
  DISABLE_TIMER
  b finalise_timer

finalise_timer:
  ldp x0, x1, [sp], #16 // restore thread's regs
  eret

semihosting:
  /* Do semihosting call
     We don't let threads hlt directly because
     a halt's exception link register is the halt,
     not the next instr. Which makes things complicated.
  */
  ldp x0, x1, [sp], #16 // restore thread's regs which hold the args
  msr SPSel, #0         // use thread's sp (points to semihosting data)
  hlt 0xf000
  eret

__thread_switch:
  /* Validate stack extent */
  ldr x0, =thread_stack_offset
  ldr x1, =_current_thread
  ldr x0, [x0]              // chase it
  ldr x1, [x1]              // chase current thread too
  add x0, x1, x0            // get minimum valid stack pointer
  msr SPSel, #0             // get the thread's sp
  mov x1, sp
  sub x1, x1, #((31+2+1)*8) // take away space we want to use
  cmp x0, x1                // is potential sp < min valid sp?
  b.hs stack_extent_failed  // Use thread's stack for this, we will exit anyway

  msr SPSel, #1
  ldp x0, x1, [sp], #16     // Restore thread's regs for saving

  msr SPSel, #0             // Switch to thread's stack (EL0_SP)

  /* Save all registers to stack */
  stp x0,  x1,  [sp, #-16]!

  mrs x0, FPSR              // Restore these second to last
  mrs x1, SPSR_EL1          // so we have temp regs x0/x1 to msr from
  stp x0, x1,   [sp, #-16]!

  /* Save the PC we are switching from */
  mrs x1, ELR_EL1

  stp x1,  x2,  [sp, #-16]! // PC included here
  stp x3,  x4,  [sp, #-16]!
  stp x5,  x6,  [sp, #-16]!
  stp x7,  x8,  [sp, #-16]!
  stp x9,  x10, [sp, #-16]!
  stp x11, x12, [sp, #-16]!
  stp x13, x14, [sp, #-16]!
  stp x15, x16, [sp, #-16]!
  stp x17, x18, [sp, #-16]!
  stp x19, x20, [sp, #-16]!
  stp x21, x22, [sp, #-16]!
  stp x23, x24, [sp, #-16]!
  stp x25, x26, [sp, #-16]!
  stp x27, x28, [sp, #-16]!
  stp x29, x30, [sp, #-16]!

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr x10, =_current_thread
  ldr x11, =next_thread

  /* Save our stack pointer */
  ldr x1, [x10]          // get actual adress of current thread
  mov x3, sp
  str x3, [x1], #8

  /* update state */
  ldr x2, [x1]
  mov x3, #running
  cmp x2, x3
  bne dont_update_state
  mov x2, #suspended
  str x2, [x1]
dont_update_state:

  /* Switch to new thread */
  ldr x11, [x11]         // chase to get actual address of the new thread
  str x11, [x10]         // _current_thread = new_thread
  ldr x3, [x11], #8      // restore stack pointer of new thread
  mov sp, x3

  /* Check that the new thread has been run at least once */
  ldr x3, [x11]
  mov x4, #init
  cmp x3, x4

  /* either way we start running */
  mov x4, #running
  str x4, [x11]

  bne restore_regs

  /* Fake return PC value (doesn't matter what reg we use here)*/
  ldr x30, =thread_start
  msr ELR_EL1, x30
  b exc_return

restore_regs:
  /* Restore all registers of the new thread */
  ldp x29, x30, [sp], #16
  ldp x27, x28, [sp], #16
  ldp x25, x26, [sp], #16
  ldp x23, x24, [sp], #16
  ldp x21, x22, [sp], #16
  ldp x19, x20, [sp], #16
  ldp x17, x18, [sp], #16
  ldp x15, x16, [sp], #16
  ldp x13, x14, [sp], #16
  ldp x11, x12, [sp], #16
  ldp x9,  x10, [sp], #16
  ldp x7,  x8,  [sp], #16
  ldp x5,  x6,  [sp], #16
  ldp x3,  x4,  [sp], #16
  ldp x1,  x2,  [sp], #16

  /* x1 = restore PC */
  msr ELR_EL1, x1

  /* This is FPSR/PSR */
  ldp x0,  x1,  [sp], #16
  msr FPSR, x0
  msr SPSR_EL1, x1

  /* Actual x0 and x1 */
  ldp x0,  x1,  [sp], #16

exc_return:
  eret
