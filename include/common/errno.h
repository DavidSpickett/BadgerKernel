#ifndef COMMON_ERRNO_H
#define COMMON_ERRNO_H

/* [[[cog
import cog
from scripts.errnos import errnos
name_len = max([len(name) for name, num, desc in errnos])
for name, num, desc in errnos:
  cog.outl("#define {} {} // {}".format(name.ljust(name_len), num, desc))
]]] */
#define E_PERM         1 // permisson denied
#define E_INVALID_ID   2 // invalid thread ID
#define E_NOT_FOUND    3 // file not found
#define E_NO_PAGE      4 // no free code page
#define E_INVALID_ARGS 5 // invalid arguments
/* [[[end]]] */

char* strerror(int errnum);

#endif /* ifdef COMMON_ERRNO_H */
