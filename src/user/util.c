#include "user/util.h"
#include "common/syscall.h"

void exit(int status) {
  DO_SYSCALL_1(exit, status);
}
