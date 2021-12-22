#ifndef COMMON_ASSERT_H
#define COMMON_ASSERT_H

#include "common/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

BK_EXPORT __attribute__((noreturn)) void __assert_fail(const char* __assertion,
                                                       const char* __file,
                                                       unsigned int __line,
                                                       const char* __function);

#ifdef NDEBUG
#define assert(expr) (void)expr
#else
#define assert(expr)                                                           \
  {                                                                            \
    if (!(expr))                                                               \
      __assert_fail(#expr, __FILE__, __LINE__, __func__);                      \
  }
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifdef COMMON_ASSERT_H */
