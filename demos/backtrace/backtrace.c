#include "common/trace.h"
#include "user/thread.h"
#include "user/util.h"
#include "util.h"

// Hacky way to get function range without
// debug info.
// Add nop before label so that any PC will be
// fn >= pc < __end_fn
#define END_SYMBOL(NAME)                                                       \
  asm volatile("nop\n\t"                                                       \
               ".global __end_" NAME "\n\t"                                    \
               "__end_" NAME ":\n\t")

void branch(void);
void setup(void);
void leaf(void);

void ccc(void) {
  branch();
  END_SYMBOL("ccc");
}
void bbb(void) {
  // If you get your fp offset wrong these show up
  int i = 0xdeadbeef;
  (void)i;
  int j = 0xcafef00d;
  (void)j;
  ccc();
  END_SYMBOL("bbb");
}
void aaa(void) {
  bbb();
  END_SYMBOL("aaa");
}

extern void* __end_leaf;
extern void* __end_branch;
extern void* __end_ccc;
extern void* __end_bbb;
extern void* __end_aaa;
extern void* __end_setup;
void backtracer() {
  const Symbol backtrace_symbols[] = {
      {"leaf", leaf, &__end_leaf}, {"branch", branch, &__end_branch},
      {"ccc", ccc, &__end_ccc},    {"bbb", bbb, &__end_bbb},
      {"aaa", aaa, &__end_aaa},    {"setup", setup, &__end_setup},
  };
  const size_t num_symbols = sizeof(backtrace_symbols) / sizeof(Symbol);

  RegisterContext ctx;
  get_thread_registers(0, &ctx);
  log_event("backtrace from branch function");
  // Technically it's from generic_syscall which is a leaf
  // The point is the user thread was in a branch function
  print_backtrace(ctx, backtrace_symbols, num_symbols);

  yield();

  get_thread_registers(0, &ctx);
  log_event("backtrace from leaf function");
  print_backtrace(ctx, backtrace_symbols, num_symbols);
}

void leaf(void) {
  // Use direct yield here so that it remains a leaf function
  asm volatile("svc %0" : : "i"(svc_thread_switch) : "memory");
  END_SYMBOL("leaf");
}

void do_nothing(void) {}
void branch(void) {
  // Can't rely on lr during a function
  do_nothing();
  yield();
  // Note: you can't backtrace yourself because
  // the frame info for the old frames will have
  // been overwritten by you calling the backtrace
  // function.

  leaf();

  // TODO: Can't backtrace yourself because thread
  // registers aren't fully saved on syscalls
  END_SYMBOL("branch");
}

void setup(void) {
  set_thread_name(-1, "tracee");
  add_named_thread(backtracer, "backtracer");
  aaa();
  END_SYMBOL("setup");
}
