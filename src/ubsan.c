#include "semihosting.h"
#include "print.h"

void __ubsan_handle_add_overflow_abort(void* a, void* b, void* c) { 
  print("UBSAN: add_overflow_abort\n");
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_out_of_bounds_abort(void* a, void* b) { 
  print("UBSAN: out_of_bounds_abort\n"); 
  qemu_exit(); 
  __builtin_unreachable();
}

void __ubsan_handle_type_mismatch_abort(void* a, void* b) { 
  print("UBSAN: type_mismatch_abort\n");
  qemu_exit();
  __builtin_unreachable();
}

void __ubsan_handle_builtin_unreachable(void* a) {
  qemu_print("UBSAN: builtin_unreachable\n");
  qemu_exit(); 
  __builtin_unreachable();
}

void __ubsan_handle_load_invalid_value_abort(void* a, void* b) {
  qemu_print("UBSAN: load_invalid_value_abort\n"); 
  qemu_exit(); 
  __builtin_unreachable(); 
}
