#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "semihosting.h"
#include "print.h"

struct SourceInfo {
  const char* filename;
  uint32_t line;
  uint32_t column;
};

// unsigned base 10 only
void uint_to_str(uint32_t n, char* out) {
  char* start = out;

  uint32_t div = 10;
  // Luckily line/col starts from 1
  while (n) {
    uint32_t digit = n % div;
    *out++ = ((char)digit)+48;
    n /= div;
  }

  // Now reverse the digits
  size_t len = strlen(start);
  --out;
  for (size_t idx = 0; idx != (len/2); ++idx) {
    char c = start[idx];
    *(start+idx) = *(out-idx);
    *(out-idx) = c;
  }
}

void print_source_info(struct SourceInfo* s) {
  // 10 digits of lines/columns should be enough for anyone
  char nums[11];
  memset(nums, 0, 10);

  print(s->filename);
  print(":");

  uint_to_str(s->line, nums);
  print(nums);

  print(":");

  memset(nums, 0, 10);
  uint_to_str(s->column, nums);
  print(nums);

  print("\n");
}

// Attribute used for when we're using LTO
#define ubhandler(NAME, ...) \
__attribute__((used)) \
void __ubsan_handle_##NAME(struct SourceInfo* s, ##__VA_ARGS__) { \
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
