#ifndef USER_ERRNO_H
#define USER_ERRNO_H

#include "common/errno.h"

#define errno (*__get_errno())
int* __get_errno(void);

#endif /* ifdef USER_ERRNO_H */
