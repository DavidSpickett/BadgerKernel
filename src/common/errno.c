#include "common/errno.h"

char* strerror(int errnum) {
  /* [[[cog
  import cog
  from scripts.errnos import errnos
  cog.outl("switch (errnum) {")
  for name, num, desc in errnos:
    cog.outl("  case {}:".format(name))
    cog.outl("    return \"{}\";".format(desc))
  ]]] */
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
      /* [[[end]]] */
    default:
      return "<unknown errno>";
  }
}
