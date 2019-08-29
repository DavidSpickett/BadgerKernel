#include <stdint.h>
#include <string.h>
#include "semihosting.h"
#include "print.h"

typedef struct {
  const char* filename;
  uint32_t line;
  uint32_t column;
} SourceInfo;

void print_source_info(SourceInfo* info) {
  char line_no[5];
  uint_to_str(info->line, line_no);
  char col_no[5];
  uint_to_str(info->column, col_no);

  print("%s:%s:%s\n", info->filename, line_no, col_no);
}

// Attribute used for when we're using LTO
#define ubhandler(NAME, ...) \
__attribute__((used)) \
void __ubsan_handle_##NAME(SourceInfo* s, ##__VA_ARGS__) { \
  print("UBSAN: " #NAME " @ "); \
  print_source_info(s); \
  qemu_exit(); \
  __builtin_unreachable(); \
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
