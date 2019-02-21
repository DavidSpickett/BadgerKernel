#include "semihosting.h"
#include "print.h"

void __ubsan_handle_add_overflow(void* a, void* b, void* c) {
  print("UBSAN: add_overflow\n");
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_out_of_bounds(void* a, void* b) {
  print("UBSAN: out_of_bounds\n");
  qemu_exit(); 
  __builtin_unreachable();
}

void __ubsan_handle_type_mismatch(void* a, void* b) {
  print("UBSAN: type_mismatch\n");
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_builtin_unreachable(void* a) {
  qemu_print("UBSAN: builtin_unreachable\n");
  qemu_exit(); 
  __builtin_unreachable();
}

void __ubsan_handle_load_invalid_value(void* a, void* b) {
  qemu_print("UBSAN: load_invalid_value\n");
  qemu_exit(); 
  __builtin_unreachable(); 
}

void __ubsan_handle_nonnull_arg(void* a, void* b) {
  qemu_print("UBSAN: handle_nonnull_arg\n");
  qemu_exit();
  __builtin_unreachable();
}
