#include "thread_state.h"

.set TIMER_AMOUNT, 100000

.macro CHECK_SVC code, handler
  mov x1, #\code
  cmp x0, x1
  beq \handler
.endm

.macro CURRENT_IN_X20_NEXT_IN_X21
  // Deliberatley callee saved so we can call the scheduler easily
  ldr x20, =_current_thread
  ldr x21, =next_thread
.endm

.macro DISABLE_TIMER
  mov x0, #2                 // Disable timer and mask interrupt
  msr CNTV_CTL_EL0, x0
.endm

.macro SAVE_KERNEL_TEMPS
  msr SPSel, #1             // Use kernel stack
  // All caller saved registers
  stp x0,  x1,  [sp, #-16]!
  stp x2,  x3,  [sp, #-16]!
  stp x4,  x5,  [sp, #-16]!
  stp x6,  x7,  [sp, #-16]!
  stp x8,  x9,  [sp, #-16]!
  stp x10, x11, [sp, #-16]!
  stp x12, x13, [sp, #-16]!
  stp x14, x15, [sp, #-16]!
  stp x16, x17, [sp, #-16]!
  stp x18, x29, [sp, #-16]!
  stp x30, xzr, [sp, #-16]!
.endm

.macro POP_KERNEL_TEMPS
  ldp x30, xzr, [sp], #16
  ldp x18, x29, [sp], #16
  ldp x16, x17, [sp], #16
  ldp x14, x15, [sp], #16
  ldp x12, x13, [sp], #16
  ldp x10, x11, [sp], #16
  ldp x8,  x9,  [sp], #16
  ldp x6,  x7,  [sp], #16
  ldp x4,  x5,  [sp], #16
  ldp x2,  x3,  [sp], #16
  ldp x0,  x1,  [sp], #16
.endm

.macro POP_KERNEL_TEMPS_USE_EL0_SP
  POP_KERNEL_TEMPS
  msr SPSel, #0
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

  /* Set next to NULL to run the scheduler */
  mov x0, #0
  ldr x1, =next_thread
  str x0, [x1] // next_thread = NULL

  b check_stack_extent

.global handle_svc
handle_svc:
  SAVE_KERNEL_TEMPS
  /* See what brought us here. */
  mrs x0, ESR_EL1
  lsr x0, x0, #26    // check exception code
  mov x1, #0x15      // SVC
  cmp x0, x1
  beq check_svc
  b .                // unknown source

check_svc:
  mrs x0, ESR_EL1    // Reload then check svc code
  mov x1, #0xFFFF    // mask to get code
  and x0, x0, x1
  CHECK_SVC svc_thread_switch, check_stack_extent
  CHECK_SVC svc_enable_timer, enable_timer
  CHECK_SVC svc_disable_timer, disable_timer
  CHECK_SVC svc_syscall, generic_syscall
  b .                // unknown SVC

generic_syscall:
  // x20 = operation number
  ldr x1, =syscall_handlers
  lsl x20, x20, #3 // Shift 3 = *8, 8 bytes per address
  add x20, x20, x1
  ldr x20, [x20]

  // Get back argument registers
  POP_KERNEL_TEMPS
  SAVE_KERNEL_TEMPS

  blr x20     // Call handler
  mov x20, x0 // Temporarily save result

  // If next_thread is NULL by now, then just return normally
  mov x0, #0
  ldr x1, =next_thread
  ldr x1, [x1]
  cmp x0, x1
  beq syscall_ret

  // Otherwise we need to do a thread switch
  POP_KERNEL_TEMPS
  mov x0, x20 // Patch in result for eventual resume
  SAVE_KERNEL_TEMPS // Will get unstacked again later
  b check_stack_extent

syscall_ret:
  POP_KERNEL_TEMPS
  mov x0, x20 // Patch back in result
  eret

syscall_handlers:
#include "common/syscall.h"

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
finalise_timer:
  POP_KERNEL_TEMPS
  eret

.global load_first_thread
load_first_thread:
  // Kernel calls this so no current thread to save
  CURRENT_IN_X20_NEXT_IN_X21
  b load_next_thread

check_stack_extent:
  ldr x0, =thread_stack_offset // x0 = &thread_stack_offset
  ldr x1, =_current_thread     // x1 = &_current_thread
  ldr x0, [x0]                 // x0 = thread_stack_offset
  ldr x1, [x1]                 // x1 = _current_thread
  add x0, x1, x0               // get minimum valid stack pointer
  msr SPSel, #0                // get thread's stack pointer
  mov x1, sp
  msr SPSel, #1                // back to kernel stack
  sub x1, x1, #((31+2+1)*8)    // take away space we want to use
  cmp x0, x1                   // is potential sp < min valid sp?
  bhs stack_extent_failed      // call C function to error and exit

save_current_thread:
  POP_KERNEL_TEMPS_USE_EL0_SP

  /* Save all registers to user stack */
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

  CURRENT_IN_X20_NEXT_IN_X21

  /* Save stack pointer */
  ldr x1, [x20]        // x1 = _current_thread
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
  // If next_thread is null then we need to run the scheduler
  ldr x6, [x21]        // x0 = next_thread
  mov x1, #0
  cmp x6, x1
  bne actually_load_thread

  // Otherwise pick a new thread and do housekeeping
  msr SPSel, #1              // Use kernel's stack
  bl do_scheduler            // This will set next_thread
  ldr x6, [x21]              // Get new next_thread

actually_load_thread:
  /* Either we ran the code above, or kernel jumped here
     to run setup_thread. Either way we want to set EL0_SP */
  msr SPSel, #0            // Back to user stack
  str x6, [x20]            // _current_thread = next_thread
  mov x12, #0              // Set next to null for next switch to call scheduler
  str x12, [x21]           // next_thread = NULL
  ldr x20, [x20]           // x20 = _current_thread
  ldr x3, [x20], #8        // x3 = _current_thread->stack_ptr
  mov sp, x3

  mov x4, #running         // start running
  str x4, [x20]            // _current_thread->state = running

  /* Restore all registers of the new thread
     (even init state threads have a blank context
      to restore)
  */
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

  /* This is FPSR/SPSR */
  ldp x0, x1, [sp], #16
  msr FPSR, x0
  msr SPSR_EL1, x1

  /* Actual x0 and x1 */
  ldp x0, x1, [sp], #16

exception_return:
  eret
