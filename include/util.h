#ifndef UTIL_H
#define UTIL_H

#include "print.h"
#include <stdbool.h>

void k_exit(int status);

/* Turns out, OCLint will complain about the macros
   in <assert.h> as well! Hooray. So we'll make our
   own macro, with solitaire and diet coke.
*/
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
