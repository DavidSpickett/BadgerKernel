#ifndef COMMON_THREAD_H
#define COMMON_THREAD_H

#include <stddef.h>
#include <stdbool.h>

// Thread related stuff available to user and kernel

#define INVALID_THREAD -1
// For the set/get property API
#define CURRENT_THREAD INVALID_THREAD

#define KCFG_DESTROY_ON_STACK_ERR 1<<0
#define KCFG_LOG_SCHEDULER        1<<1
#define KCFG_LOG_THREADS          1<<2

#define TPROP_ID    0
#define TPROP_NAME  1
#define TPROP_CHILD 2
#define TPROP_STATE 3

#define THREAD_FUNC 0
#define THREAD_FILE 1

#define YIELD_ANY  0
#define YIELD_TO   1
#define YIELD_NEXT 2

typedef struct {
  size_t a1;
  size_t a2;
  size_t a3;
  size_t a4;
} ThreadArgs;

// Note the () around the args so we don't mess up pointer
// arithmetic e.g. make_args(argv+1...) => (size_t)(argv+1)
#define make_args(a, b, c, d)                   \
  { (size_t)(a), (size_t)(b), (size_t)(c), (size_t)(d) }

#endif /* ifdef COMMON_THREAD_H */
