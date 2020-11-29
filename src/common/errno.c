#include "common/errno.h"

char* strerror(int errnum) {
  // TODO: cog this
  switch (errnum) {
    case E_PERM:
      return "permission denied";
    case E_INVALID_ID:
      return "invalid thread ID";
    case E_NOT_FOUND:
      return "file not found";
    case E_NO_PAGE:
      return "no free code page";
    case E_INVALID_ARGS:
      return "invalid arguments";
    default:
      return "(unknown errno)";
  }
}
