#include <stdint.h>
#include "semihosting.h"
#include "print.h"
#include "util.h"

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

void __ubsan_handle_add_overflow(struct SourceInfo* s, void* b, void* c) {
  print("UBSAN: add_overflow @ ");
  print_source_info(s);
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_out_of_bounds(struct SourceInfo* s, void* b) {
  print("UBSAN: out_of_bounds @ ");
  print_source_info(s);
  qemu_exit(); 
  __builtin_unreachable();
}

void __ubsan_handle_type_mismatch(struct SourceInfo* s, void* b) {
  print("UBSAN: type_mismatch @ ");
  print_source_info(s);
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_builtin_unreachable(struct SourceInfo* s) {
  qemu_print("UBSAN: builtin_unreachable @ ");
  print_source_info(s);
  qemu_exit(); 
  __builtin_unreachable();
}

void __ubsan_handle_load_invalid_value(struct SourceInfo* s, void* b) {
  qemu_print("UBSAN: load_invalid_value @ ");
  print_source_info(s);
  qemu_exit(); 
  __builtin_unreachable(); 
}

void __ubsan_handle_nonnull_arg(struct SourceInfo* s, void* b) {
  qemu_print("UBSAN: handle_nonnull_arg @ ");
  print_source_info(s);
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_divrem_overflow(struct SourceInfo* s, void* b, void* c) {
  qemu_print("UBSAN: divrem_overflow @ ");
  print_source_info(s);
  qemu_exit();
  __builtin_unreachable();
}
