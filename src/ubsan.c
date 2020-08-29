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
    while (1) {                                                                \
    }                                                                          \
    __builtin_unreachable();                                                   \
  }
/* Using a while(1) to stall here and lit will kill the test after some time.
   I used to exit() but since that now goes through the kernel there's no way to
   know whether we're already in the kernel. If we call normal exit() from
   kernel bad things happen. (things that are harder to fix than adding a lit
   timeout)
*/

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
ubhandler(type_mismatch_v1, void* a, void* b);
ubhandler(pointer_overflow, void* a, void* b, void* c);
