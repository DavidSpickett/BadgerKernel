#include "user/thread.h"
#include "user/fibre.h"

void do_work(UserContext* other) {
  log_event("EFGH");
  swap_context(other);

  log_event("LMNO");
  swap_context(other);

  log_event("TUVW");
  // Note that this returns normally
}

void test_swap(void) {
  UserContext ctx;
  // Arm O0 = 512
  // AArch64 O0 = 1024
  size_t do_work_stack_size = 1024;
  uint8_t do_work_stack[do_work_stack_size];
  make_context(&ctx, do_work, &do_work_stack[do_work_stack_size]);

  log_event("ABCD");
  swap_context(&ctx);

  log_event("HIJK");
  swap_context(&ctx);

  log_event("PQRS");
  // One last swap so that do_work returns normally
  swap_context(&ctx);

  // Which is caught and redirected back here
  log_event("XYZ!");
}

void setup(void) {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  volatile int num = 0;

  UserContext ctx;
  get_context(&ctx);

  if (num) {
    log_event("Hello fibres again!");
  } else {
    log_event("Hello fibres!");
  }

  if (!num) {
    num++;
    set_context(&ctx);
  }

  test_swap();
}
