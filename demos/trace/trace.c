#include "common/trace.h"
#include "common/assert.h"
#include "common/attribute.h"
#include "port/port.h"
#include "user/thread.h"
#include "user/util.h"

extern void work_finished(void);

static void set_thread_pc(int tid, void* pc) {
  RegisterContext ctx;
  get_thread_registers(tid, (RegisterContext*)&ctx);
  // Remove bottom bit for thumb
  ctx.generic_regs.pc = PC_REMOVE_MODE((size_t)pc);
  set_thread_registers(tid, ctx);
}

void jump_here() {
  log_event("Changed registers!");
  // No point returning because our LR will be bogus
  thread_wait();
}

void pre_jumped() {
  // Will be redirected by setup
  __builtin_unreachable();
}

void jump_self() {
  // This proves that you can modify your own registers
  // since the thread is saved before every syscall.
  set_thread_pc(CURRENT_THREAD, jump_here);
  __builtin_unreachable();
}

void tracee() {
  log_event("Working...");
  yield();
  work_finished();
}

// Note: the original plan was to write this svc in manually
// However this code will be in ROM so that couldn't be done.

ATTR_NAKED __attribute__((used)) void __work_finished(void) {
  // By defining work_finished in assembly we can be sure of it's address
  asm volatile(".global work_finished\n\t"
               "work_finished:\n\t"
               "svc %0\n\t"
               // If the tracer doesn't redirect us we'll loop forever
               "b tracee\n\t"
               :
               : "i"(svc_thread_switch));
}

void finish_program(void) {
  log_event("Finished program");
  exit(0);
}

void tracer() {
  int tid = add_named_thread(tracee, "tracee");
  set_child(tid);

  RegisterContext ctx;

  size_t target_pc = (size_t)work_finished;
  // Stored PC doesn't include mode bit
  target_pc = PC_REMOVE_MODE(NEXT_INSTR(target_pc));

  while (ctx.generic_regs.pc != target_pc) {
    yield();
    get_thread_registers(tid, &ctx);
  }
  log_event("Hit work_finished!");

  // Now redirect to prevent infinite loop
  ctx.generic_regs.pc = (size_t)finish_program;
  assert(set_thread_registers(tid, ctx));
  log_event("Redirected tracee");
  yield();
}

void setup(void) {
  set_thread_name(CURRENT_THREAD, "setup");

  // Has registers changed even before running
  int tid = add_named_thread(pre_jumped, "prejumped");
  set_thread_pc(tid, jump_here);
  yield();

  // Changes its own registers
  add_named_thread(jump_self, "jumpself");
  yield();
  add_named_thread(tracer, "tracer");
}
