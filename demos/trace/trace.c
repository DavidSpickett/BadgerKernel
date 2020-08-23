#include "user/thread.h"
#include "user/util.h"
#include "util.h"
#include "common/trace.h"

extern void work_finished(void);

void tracee() {
  log_event("Working...");
  yield();
  work_finished();
}

// Note: the original plan was to write this svc in manually
// However this code will be in ROM so that couldn't be done.

__attribute__((naked, used))
void __work_finished(void) {
  // By defining work_finished in assembly we can be sure of it's address
  asm volatile(
    ".global work_finished\n\t"
    "work_finished:\n\t"
    "svc %0\n\t"
    // If the tracer doesn't redirect us we'll loop forever
    "b tracee\n\t"
    : : "i"(svc_thread_switch));
}

void finish_program(void) {
  log_event("Finished program");
  exit(0);
}

void tracer() {
  int tid = add_named_thread(tracee, "tracee");
  set_child(tid);

  // // Can't set/get regs for an init state
  RegisterContext ctx;
  // assert(!get_thread_registers(tid, &ctx));
  // assert(!set_thread_registers(tid, ctx));

  // // Get out of init state
  // yield();

  size_t target_pc = (size_t)work_finished;
#ifdef __thumb__
  // +2 for next instr
  target_pc += 2;
  // Stored PC doesn't include mode bit so remove it
  // TODO: why is that?
  target_pc &= ~1;
#else
  target_pc += 4;
#endif

  while(ctx.pc != target_pc) {
    yield();
    get_thread_registers(tid, &ctx);
  }
  log_event("Hit work_finished!");

  // Now redirect to prevent infinite loop
  ctx.pc = (size_t)finish_program;
  assert(set_thread_registers(tid, ctx));
  log_event("Redirected tracee");
  yield();
}

void setup(void) {
  add_named_thread(tracer, "tracer");
}
