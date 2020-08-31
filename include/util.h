#ifndef UTIL_H
#define UTIL_H

#include "common/print.h"
#include <stdbool.h>

void k_exit(int status);

__attribute__((noreturn)) void __assert_fail(const char* __assertion,
                                             const char* __file,
                                             unsigned int __line,
                                             const char* __function);

#ifdef NDEBUG
#define assert(expr) (void)expr
#else
#define assert(expr)                                                           \
  {                                                                            \
    bool condition = expr;                                                     \
    condition ? 0 : __assert_fail(#expr, __FILE__, __LINE__, __func__);        \
  }
#endif

#endif /* ifdef UTIL_H */
