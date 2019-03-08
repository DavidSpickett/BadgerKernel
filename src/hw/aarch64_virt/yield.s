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

.global platform_yield_initial
platform_yield_initial:
  /* Called when starting scheduler (trashing regs is fine) */
  ldr x0, =current_thread // init stack pointer to
  ldr x0, [x0]            // stack pointer of dummy thread
  ldr x0, [x0]            // so we can pass the check normally
  mov sp, x0
  bl platform_yield

.global platform_yield
platform_yield:
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

.global thread_switch
thread_switch:
  CHECK_MONITOR_STACK
1:
  /* See what brought us here. */
  mrs x0, ESR_EL1
  lsr x0, x0, #26    // check exception code
  mov x1, #0x15      // SVC
  cmp x0, x1
  beq handle_svc

  b unknown_exc

handle_svc:
  mrs x0, ESR_EL1    // Reload then check svc code
  mov x1, #0xFFFF    // mask svc number
  and x0, x0, x1
  mov x1, #0xdead    // thread switch
  cmp x0, x1
  beq __thread_switch
  mov x1, #0x3333    // semihosting
  cmp x0, x1
  beq semihosting
  mov x1, #1         // enable virtual timer
  cmp x0, x1
  beq enable_timer
  mov x1, #0         // disable virtual timer
  cmp x0, x1
  beq disable_timer

  b unknown_exc

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
  ldr x1, =current_thread
  ldr x0, [x0]              // chase it
  ldr x1, [x1]              // chase current thread too
  add x1, x1, x0            // get minimum valid stack pointer
  msr SPSel, #0             // get the thread's sp
  mov x0, sp
  sub x0, x0, #((31+2)*64)  // take away space we want to use
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

  stp x2,  x3,  [sp, #-16]!
  stp x4,  x5,  [sp, #-16]!
  stp x6,  x7,  [sp, #-16]!
  stp x8,  x9,  [sp, #-16]!
  stp x10, x11, [sp, #-16]!
  stp x12, x13, [sp, #-16]!
  stp x14, x15, [sp, #-16]!
  stp x16, x17, [sp, #-16]!
  stp x18, x19, [sp, #-16]!
  stp x20, x21, [sp, #-16]!
  stp x22, x23, [sp, #-16]!
  stp x24, x25, [sp, #-16]!
  stp x26, x27, [sp, #-16]!
  stp x28, x29, [sp, #-16]!
  stp x30, xzr, [sp, #-16]!

  /* Setup pointers in some high reg numbers we won't overwrite */
  ldr x10, =current_thread
  ldr x11, =next_thread

  /* Save our PC and return address */
  ldr x1, [x10]          // get actual adress of current thread
  mov x3, sp
  str x3, [x1], #8       // save current stack pointer
  mrs x2, ELR_EL1        // get address to resume this thread from
  str x2, [x1]           // store that in our task struct

  /* Switch to new thread */
  ldr x11, [x11]         // chase to get actual address of the new thread
  str x11, [x10]         // current_thread = new_thread
  ldr x3, [x11], #8      // restore stack pointer of new thread
  mov sp, x3

  // Load ELR_EL1 with the next PC of the new thread
  ldr x3, [x11]
  msr ELR_EL1, x3        // when we eret we'll be in the new thread

  /* Check that the new thread has been run at least once.
     If it hasn't then there's no state to restore.
     For now we're going to check if it's current PC is thread_start
  */
  ldr x4, =thread_start
  cmp x3, x4
  beq exc_return

  /* Restore all registers of the new thread */
  ldp x30, xzr, [sp], #16 // xzr to keep alignment
  ldp x28, x29, [sp], #16
  ldp x26, x27, [sp], #16
  ldp x24, x25, [sp], #16
  ldp x22, x23, [sp], #16
  ldp x20, x21, [sp], #16
  ldp x18, x19, [sp], #16
  ldp x16, x17, [sp], #16
  ldp x14, x15, [sp], #16
  ldp x12, x13, [sp], #16
  ldp x10, x11, [sp], #16
  ldp x8,  x9,  [sp], #16
  ldp x6,  x7,  [sp], #16
  ldp x4,  x5,  [sp], #16
  ldp x2,  x3,  [sp], #16

  ldp x0,  x1,  [sp], #16 // This is FPSR/PSR not actually x0/x1
  msr FPSR, x0
  msr SPSR_EL1, x1

  ldp x0,  x1,  [sp], #16 // actual x0/x1 values

exc_return:
  // Return to PC that's in ELR_EL1
  eret
