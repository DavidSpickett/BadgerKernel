#include "common/errno.h"
#include "user/thread.h"

int* __get_errno(void) {
  int* errno_ptr = NULL;
  get_thread_property(-1, TPROP_ERRNO_PTR, (size_t*)&errno_ptr);
  return errno_ptr;
}
