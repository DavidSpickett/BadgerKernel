#include "common/assert.h"
#include "common/print.h"
#include "user/util.h"

__attribute__((noreturn)) void __assert_fail(const char* __assertion,
                                             const char* __file,
                                             unsigned int __line,
                                             const char* __function) {
  printf("%s:%u (%s) Expected: %s\n", __file, __line, __function, __assertion);
  exit(1);
  __builtin_unreachable();
}
