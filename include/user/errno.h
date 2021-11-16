#ifndef USER_ERRNO_H
#define USER_ERRNO_H

#include "common/errno.h"
#include "common/macros.h"

#define errno (*__get_errno())
BK_EXPORT int* __get_errno(void);

#endif /* ifdef USER_ERRNO_H */
