#include "user/errno.h"
#include "user/thread.h"

int* __get_errno(void) {
  return &user_thread_info.err_no;
}
