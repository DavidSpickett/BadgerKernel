#include "common/errno.h"
#include "user/thread.h"

int* __get_errno(void) {
  // volatile to fix issues at O3 with LTO
  int* volatile errno_ptr = NULL;
  get_thread_property(-1, TPROP_ERRNO_PTR,
    (size_t*)&errno_ptr);
  return errno_ptr;
}
