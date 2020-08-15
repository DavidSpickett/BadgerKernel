#include "user/thread.h"
#include "thread.h"
#include "user/util.h"
#include "util.h"
#include "common/trace.h"

void work_finished();

void tracee() {
  log_event("Working...");
  yield();
  work_finished();
}

// Note: the original plan was to write this svc in manually.// However this code will be in ROM so that couldn't be done.

__attribute__((naked)) void work_finished(void) {
  // Need naked so the address of this instr is the same
  // as the address of the function
  asm volatile(
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

  // Can't set/get regs for an init state
  RegisterContext ctx;
  assert(!get_thread_registers(tid, &ctx));
  assert(!set_thread_registers(tid, ctx));

  // Get out of init state
  yield();

  // +4 as the pc will be on the next instruction
  size_t target_pc = ((size_t)work_finished)+4;
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
  K_ADD_NAMED_THREAD(tracer, "tracer");
}
