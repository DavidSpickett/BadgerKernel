#include "user/thread.h"
#include "user/util.h"
#include "util.h"
#include "common/trace.h"

void other() {
  RegisterContext ctx;
  assert(get_thread_registers(0, &ctx));
  print_register_context(ctx);
  print_backtrace(ctx);
  exit(0);
}

void do_more_nothing(void) {
}
void do_nothing(void) {
  do_more_nothing();
}

void bar(void) {
  int i = 0xdeadbeef; (void)i;
  int j = 0xdeadbeef; (void)j;
  //log_event("hey!");
  //yield();
  //do_nothing(); // corrupt lr register?
 asm volatile("svc %0" : : "i"(svc_thread_switch) : "memory");
  // Would backtrace here but you can't read your own
  // regs as they aren't fully saved when you just
  // syscall.
}
void zzz(void) {
  bar();
}
void baz(void) {
  // If you get your fp offset wrong these show up
  int i = 0xdeadbeef;
  int j = 0xcafef00d;
  (void)i;
  (void)j;
  zzz();
}
void foo(void) {
  baz();
}

void setup(void) {
  assert(add_thread_from_worker(other) != -1);
  foo();
}
