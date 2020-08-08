#ifndef THREAD_COMMON_H
#define THREAD_COMMON_H

#include <stddef.h>
#include <stdbool.h>

// Thread related stuff available to user and kernel

typedef struct {
  bool destroy_on_stack_err;
  bool log_scheduler;
  bool log_threads;
} KernelConfig;

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

#endif /* ifdef THREAD_COMMON_H */
