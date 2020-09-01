#include "user/util.h"
#include "user/syscall.h"

void exit(int status) {
  DO_SYSCALL_1(exit, status);
}
