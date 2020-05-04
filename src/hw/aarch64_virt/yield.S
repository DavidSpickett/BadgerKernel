#include "thread_state.h"

.set TIMER_AMOUNT, 100000

.macro DISABLE_TIMER
  mov x0, #2                 // Disable timer and mask interrupt
  msr CNTV_CTL_EL0, x0
.endm

.macro SAVE_KERNEL_TEMPS
  msr SPSel, #1
  /* Use the kernel stack, we can't trust the thread stack here */
  stp x0, x1, [sp, #-16]!
.endm

.macro CURRENT_IN_X10_NEXT_IN_X11
  ldr x10, =_current_thread
  ldr x11, =next_thread
.endm

.global thread_switch
thread_switch:
  svc svc_thread_switch
  ret

/* Having this as a seperate handler is easier than
   finding the exact right register to read.
   Since I'm not sure what would happen if there
   were a pending timer int, and we happened to hit
   an SVC at the same time. We might lose the SVC.
*/
.global handle_timer
handle_timer:
  SAVE_KERNEL_TEMPS
  DISABLE_TIMER

  /* Next thread is always the scheduler on interrupt */
  mov x0, #0
  ldr x1, =next_thread
  str x0, [x1] // next_thread = NULL

  b start_thread_switch

.macro CHECK_SVC code, handler
  mov x1, #\code
  cmp x0, x1
  beq \handler
.endm

.macro POP_MONITOR_TEMPS_USE_EL0_SP
  ldp x0, x1, [sp], #16
  msr SPSel, #0
.endm

.global handle_svc
handle_svc:
  SAVE_KERNEL_TEMPS
  /* See what brought us here. */
  mrs x0, ESR_EL1
  lsr x0, x0, #26    // check exception code
  mov x1, #0x15      // SVC
  cmp x0, x1
  beq check_svc
  /* unknown source */
  b .

check_svc:
  mrs x0, ESR_EL1    // Reload then check svc code
  mov x1, #0xFFFF    // mask to get code
  and x0, x0, x1
  CHECK_SVC svc_thread_switch, start_thread_switch
  CHECK_SVC svc_semihosting, semihosting
  CHECK_SVC svc_enable_timer, enable_timer
  CHECK_SVC svc_disable_timer, disable_timer
  /* unknown svc */
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
  POP_MONITOR_TEMPS_USE_EL0_SP
  eret

semihosting:
  /* Do semihosting call
     We don't let threads hlt directly because
     a halt's exception link register is the halt,
     not the next instr. Which makes things complicated.
  */
  /* TODO: this works from kernel mode because
     semihosting doesn't use the stack pointer.
     When we re-enter kernel mode we save x0/x1 to kernel
     stack, then reload them here. So they're correct. */
  // TODO: previous SPsel *seems* to be restored on eret
  POP_MONITOR_TEMPS_USE_EL0_SP // Thread's registers points to arguments
  hlt 0xf000
  eret

.global start_thread_switch
start_thread_switch:
  /* If the current thread has never run, don't save anything.
     Used for initial scheduler start and recovering from stack
     boundary issues. */
  ldr x0, =_current_thread // x0 = &_current_thread
  ldr x0, [x0]             // x0 = _current_thread
  mov x1, #0               // nullptr means we just started up
  cmp x0, x1
  bne check_state
  // Don't need to do anything with temps, already in kernel mode
  CURRENT_IN_X10_NEXT_IN_X11
  // Just go to user stack
  msr SPSel, #0             // get thread's stack pointer
  b load_next_thread
check_state:
  ldr x0, [x0, #4]         // x0 = _current_thread->state
  mov x1, #init
  cmp x0, x1
  bne check_stack_extent
  POP_MONITOR_TEMPS_USE_EL0_SP
  CURRENT_IN_X10_NEXT_IN_X11
  b load_next_thread

check_stack_extent:
  ldr x0, =thread_stack_offset
  ldr x1, =_current_thread
  ldr x0, [x0]              // x0 = thread_stack_offset
  ldr x1, [x1]              // x1 = _current_thread
  add x0, x1, x0            // get minimum valid stack pointer
  msr SPSel, #0             // get thread's stack pointer
  mov x1, sp
  sub x1, x1, #((31+2+1)*8) // take away space we want to use
  cmp x0, x1                // is potential sp < min valid sp?
  bhs stack_extent_failed  // call C function to error and exit

save_current_thread:
  msr SPSel, #1
  POP_MONITOR_TEMPS_USE_EL0_SP

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

  CURRENT_IN_X10_NEXT_IN_X11

  /* Save stack pointer */
  ldr x1, [x10]        // x1 = _current_thread
  mov x3, sp
  str x3, [x1], #8     // _current_thread->stack_ptr=sp

  /* Update state */
  ldr x2, [x1]         // x2 = _current_thread->state
  mov x3, #running
  cmp x2, x3           // if we're something other than running, leave it as it is
  bne load_next_thread
  mov x2, #suspended   // otherwise move to suspended
  str x2, [x1]         // _current_thread->state = suspended

load_next_thread:
  ldr x11, [x11]         // x11 = next_thread
  /* null next_thread means ask the scheduler for next */
  mov x12, #0
  cmp x11, x12
  bne actually_load_thread
  msr SPSel, #1 // Use kernel's stack
  bl do_scheduler
  msr SPSel, #0
  // Now next_thread is set
  // TODO: dedupe
  CURRENT_IN_X10_NEXT_IN_X11
  // Reload it
  ldr x11, [x11]
  
actually_load_thread:
  str x11, [x10]         // _current_thread = next_thread
  mov x12, #0            // Set next to null for next switch to call scheduler
  // Get address of next_thread again
  // TODO: dedupe
  CURRENT_IN_X10_NEXT_IN_X11
  ldr x10, [x10] // Need to chase current again here
  str x12, [x11]

  ldr x3, [x10], #8      // x3 = _current_thread->stack_ptr
  mov sp, x3

  /* check that this thread has been run at least once */
  ldr x3, [x10]          // x3 = _current_thread->state
  mov x4, #init
  cmp x3, x4

  mov x4, #running      // either way it'll start running
  str x4, [x10]         // _current_thread->state = running

  /* Don't restore if it's never run before */
  bne restore_regs

  /* Fake return PC value */
  ldr x30, =thread_start
  msr ELR_EL1, x30
  b exception_return

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
  ldp x0, x1, [sp], #16
  msr FPSR, x0
  msr SPSR_EL1, x1

  /* Actual x0 and x1 */
  ldp x0, x1, [sp], #16

exception_return:
  eret
