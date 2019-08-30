#ifndef UTIL_H
#define UTIL_H

#ifdef linux
#include <stdlib.h>
#else
void exit(int status);
#endif

#endif /* ifdef UTIL_H */
