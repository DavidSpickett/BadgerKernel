#include "print.h"
#include "util.h"
#include <stdint.h>
#include <string.h>

typedef struct {
  const char* filename;
  uint32_t line;
  uint32_t column;
} SourceInfo;

// Attribute used for when we're using LTO
#define ubhandler(NAME, ...)                                                   \
  __attribute__((used)) void __ubsan_handle_##NAME(SourceInfo* s,              \
                                                   ##__VA_ARGS__) {            \
    printf("UBSAN: " #NAME " @ %s:%u:%u\n", s->filename, s->line, s->column);  \
    exit(1);                                                                   \
    __builtin_unreachable();                                                   \
  }

ubhandler(divrem_overflow, void* a, void* b);
ubhandler(add_overflow, void* a, void* b);
ubhandler(sub_overflow, void* a, void* b);
ubhandler(load_invalid_value, void* a);
ubhandler(nonnull_arg, void* a);
ubhandler(builtin_unreachable);
ubhandler(type_mismatch, void* a);
ubhandler(out_of_bounds, void* a);
ubhandler(vla_bound_not_positive, void* a);
ubhandler(shift_out_of_bounds, void* a, void* b);
ubhandler(mul_overflow, void* a, void* b);
